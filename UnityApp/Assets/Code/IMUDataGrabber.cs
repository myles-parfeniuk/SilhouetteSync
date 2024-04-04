using UnityEngine;
using System.Runtime.InteropServices;
using System;
using System.Threading;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Reflection;


public class IMUDataGrabber
{
    [System.Serializable] //make the struct seriazable such that C# Marshal class can be used to convert to byte array before sending
    public struct payload_t
    {
        public byte request;        ///< The requested action by client.
        public byte response;       ///< The response from server when receiving, client when sending.
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string id;               ///< The hardware ID of the device
        public float quat_i;        ///< The quaternion component (i).
        public float quat_j;        ///< The quaternion component (j).
        public float quat_k;        ///< The quaternion component (k).
        public float quat_real;     ///< The real part of the quaternion.
        public byte accuracy;       ///< The accuracy of the quaternion.
        public ulong time_stamp;    ///< The timestamp in us.
        public byte retransmit_delay; ///< The retransmit delay of device (assigned by client)
        public float battery_voltage; ///< The current voltage of battery
        public byte power_state; ///< Power state of device (ie battery powered, plugged in and charging, plugged in and fully charged)
    }


    private enum Requests
    {
        client_discovery,    ///< Client discovery request.
        client_sample,       ///< Client sample request.
        client_tare,         ///< Client tare request.
        client_calibrate     ///< Client calibrate request.
    }

    private enum Responses
    {
        server_discovered,    ///< Server discovered response.
        server_sampling,      ///< Server sampling response.
        server_failure,       ///< Server failure response.
        server_success,       ///< Server success response.
        server_busy,          ///< Server busy response.
        client_affirmative,   ///< Client affirmative response.
        no_resp               ///< No response.
    }

    private enum PowerStates
    {
        USB_powered_charging, ///<USB cable connected and battery charging
        USB_powered_fully_charged, ///< USB cable connected and fully charged
        battery_powered ///< Running on battery power
    }

    public const int DISCOVERY_RETRY_COUNT = 15; ///< Number of retries for device discovery.
    private const int MAX_FRAMES_DROPPED = 60; ///<Maximum amount of frames dropped by a sensor in a row before it is considered disconnected.
    private const int TIMEOUT_MS = 15; ///< Timeout value in milliseconds for transmission (all devices successfully sending)
    private const int PORT = 49160; ///< Port number for communication.
    private const bool PRINT_LOGS = false; ///< Indicates whether logs should be printed or not.
    private const string TAG = "IMUDataGrabber: "; ///< Tag for identifying log messages.

    private static IMUDataGrabber instance; ///< Singleton instance of the IMUDataGrabber.
    private Thread check_data_thread_hdl; ///< Thread handle for check data thread.
    private ManualResetEvent stop_threads_evt = new ManualResetEvent(false); ///< Event to signal all threads to stop.
    private Dictionary<string, SilSyncDevice> id_to_device_dict = new Dictionary<string, SilSyncDevice>(); ///< Dictionary mapping device IDs to SilSyncDevice objects.
    private Dictionary<string, string> location_to_id_dict = new Dictionary<string, string>(); ///< Dictionary mapping body locations to device IDs.
    int successful_transmissions; ///< Updated by transceiver threads and used by check_data_thread to determine if all devices have sucessfully been read within 15ms
    private Semaphore data_ready_sem; ///< Semaphore used by transceiver threads to signal to check data thread that they have completed their send/receive cycle. 
    private readonly Mutex dictionary_mutex = new Mutex(); ///< Mutex for ensuring thread safety in dictionary operations.
    private readonly Mutex successful_transmissions_mutex = new Mutex(); ///<Mutex for ensuring thread saftey when incrementing or reseting successful transmissions variable. 


    /**
    * @brief Private constructor for initializing singleton isntance IMUDataGrabber.
    */
    private IMUDataGrabber()
    {
        init_location_to_id_dict();
    }

    /**
     * @brief Gets the singleton instance of the IMUDataGrabber.
     * @return The singleton instance of the IMUDataGrabber.
     */
    static public IMUDataGrabber get_grabber()
    {
        if (instance == null)
            instance = new IMUDataGrabber();

        return instance;
    }

    /**
    * @brief Initializes the location_to_id dict with hardware IDs and location strings.
    */
    private void init_location_to_id_dict()
    {
        location_to_id_dict.Add("chest", "corp-burger");
        location_to_id_dict.Add("leftBicep", "dream-opachki");
        location_to_id_dict.Add("leftWrist", "juicy-toaster");
        location_to_id_dict.Add("rightBicep", "gate-agressive");
        location_to_id_dict.Add("rightWrist", "imagination-wonderland");
        location_to_id_dict.Add("leftThigh", "heuristic-assault");
        location_to_id_dict.Add("leftCalf", "gorilla-birdy");
        location_to_id_dict.Add("leftFoot", "paris-flexible");
        location_to_id_dict.Add("rightThigh", "samaritan-blender");
    }

    /**
    * @brief Scans for devices and saves responding device addresses for transceiver threads.
    * @param max_devices The maximum number of devices to scan local netowork for.
    * @return The number of devices found and added to the dictionary.
    */
    public int scan_for_devices(int max_devices)
    {
        payload_t payload_out = assemble_discovery_packet();
        payload_t payload_in;
        UdpClient discovery_client = new UdpClient();
        IPEndPoint server_address = new IPEndPoint(IPAddress.Any, 0);
        byte[] payload_out_bytes = StructToBytes(payload_out);
        byte[] payload_in_bytes;

        disconnect();

        discovery_client.EnableBroadcast = true; //set broadcast to true to send UDP broadcast packets
        discovery_client.Client.ReceiveTimeout = 2000; //2s timeout for discovery

        for (int i = 0; i < DISCOVERY_RETRY_COUNT; i++)
        {
            //break from loop if max devices are discovered
            if (id_to_device_dict.Count >= max_devices)
            {
                Debug.Log(TAG + "Max devices discovered.");
                discovery_client.Close();
                break;
            }

            //send broadcast discovery request packet
            discovery_client.Send(payload_out_bytes, payload_out_bytes.Length, new IPEndPoint(IPAddress.Broadcast, PORT));

            try
            {
                //receive response 
                payload_in_bytes = discovery_client.Receive(ref server_address);
                payload_in = BytesToStruct<payload_t>(payload_in_bytes);

                try
                {
                    //check to see if ID is registered to a body location
                    string location = get_location_by_id(payload_in.id.TrimEnd('_'));
                    if (location != null)
                    {
                        //attempt to add device to dictionary, check in case it already exists within the dictionary
                        add_device_to_dict(server_address, payload_in);
                    }

                }
                catch (ArgumentException)
                {
                    Debug.Log(TAG + "An element with same device ID alrady exists.");
                }
            }
            catch (SocketException e)
            {

                if (e is SocketException se && se.SocketErrorCode == SocketError.TimedOut)
                    Debug.Log(TAG + "Broadcast receive operation timed out.");
            }


        }

        //print discovered IDs
        if (id_to_device_dict.Count > 0)
        {
            data_ready_sem = new Semaphore(0, id_to_device_dict.Count); //create semaphore for indicating when data is ready
            check_data_thread_hdl = new Thread(check_data_thread); //launch check data watcher thread
            check_data_thread_hdl.Start();

            Debug.Log(TAG + "Discovery Complete IDs Found: ");
            foreach (KeyValuePair<string, SilSyncDevice> pair in id_to_device_dict)
            {
                Debug.Log(TAG + "ID: " + pair.Key);
                //launch transceiever threads
                pair.Value.transceiver_thread_hdl = new Thread(new ParameterizedThreadStart(transceiver_thread));
                pair.Value.transceiver_thread_hdl.Start(get_location_by_id(pair.Value.id));

            }
        }



        return id_to_device_dict.Count;
    }

    /**
    * @brief Disconnects from all devices and clears the dictionary.
    */
    public void disconnect()
    {
        stop_threads_evt.Set();

        foreach (KeyValuePair<string, SilSyncDevice> pair in id_to_device_dict)
        {
            pair.Value.transceiver_thread_hdl.Join();
        }

        stop_threads_evt.Reset(); //signal all transceiver threads and check data threads to stop

        if (id_to_device_dict != null)
            id_to_device_dict.Clear();
    }

    /**
    * @brief Retrieves sensor data for a specified body location.
    * @param location The body location of the sensor.
    * @return Quaternion representing sensor data if available, otherwise an origin quaternion.
    */
    public Quaternion get_sensor_data(string location)
    {
        string id = get_id_by_location(location);

        if (id != null)
        {
            return id_to_device_dict[id].imu_heading_stable;
        }

        //device doesn't exist, return origin quaternion 
        Debug.LogError(TAG + "Get data failed, sensor never connected");
        return new Quaternion(0, 0, 0, 1);
    }


    /**
    * @brief Retrieves the connection state of the sensor for a specified body location.
    * @param location The body location of the sensor.
    * @return True if sensor is connected, otherwise false.
    */
    public bool get_sensor_connection_state(string location)
    {
        SilSyncDevice current_device = new SilSyncDevice();
        string id = get_id_by_location(location);

        if (id != null)
        {
            if (get_device_by_id(id, ref current_device))
                return current_device.connection_state;
        }
        return false;
    }

    /**
    * @brief Retrieves the color representing the connection state of the sensor for a specified body location.
    * @param location The body location of the sensor.
    * @return Color representing the connection state (green for connected, red for disconnected).
    */
    public Color get_sensor_connection_text(string location)
    {
        if (get_sensor_connection_state(location))
        {
            return Color.green;
        }
        else
        {
            return Color.red;
        }
    }

    /**
    * @brief Swaps the body locations of two sensors.
    * @param location_one The first body location to swap.
    * @param location_two The second body location to swap.
    * @return True if locations exist and were successfully swapped, otherwise false.
    */
    public bool swap_location(string location_one, string location_two)
    {
        if (location_to_id_dict.ContainsKey(location_one) && location_to_id_dict.ContainsKey(location_two))
        {
            string temp = location_to_id_dict[location_one];
            location_to_id_dict[location_one] = location_to_id_dict[location_two];
            location_to_id_dict[location_two] = temp;

            return true;
        }

        Debug.LogError(TAG + "Locations don't exist.");

        return false;
    }

    /**
    * @brief Initiates the calibration process for a sensor at the specified body location by triggigering event in respective transceiver thread.
    * @param location The body location of the sensor to calibrate.
    */
    public void calibrate_sensor(string location)
    {
        string id = get_id_by_location(location);
        SilSyncDevice device = new SilSyncDevice();

        if (id != null)
        {
            get_device_by_id(id, ref device);

            if (device != null)
            {
                device.calibration_evt.Set();
            }
        }

    }

    /**
    * @brief Initiates the tare process for a sensor at the specified body location by triggigering event in respective transceiver thread.
    * @param location The body location of the sensor to calibrate.
    */
    public void tare_sensor(string location)
    {
        string id = get_id_by_location(location);
        SilSyncDevice device = new SilSyncDevice();

        if (id != null)
        {
            get_device_by_id(id, ref device);

            if (device != null)
            {
                device.tare_evt.Set();
            }
        }
    }

    /**
    * @brief Retrieves the hardware ID of a sensor by its body location.
    * @param location The body location of the sensor.
    * @return The ID of the sensor if found, otherwise 0.
    */
    private string get_id_by_location(string location)
    {
        string id = null;

        if (location_to_id_dict.ContainsKey(location))
        {
            id = location_to_id_dict[location];
        }
        else
        {
            Debug.LogError(TAG + "Body location does not exist.");
        }

        return id;
    }

    /**
    * @brief Retrieves the location of a sensor by its hardware ID.
    * @param id The hardware ID of the sensor.
    * @return The location of the sensor if found, otherwise null.
    */
    private string get_location_by_id(string id)
    {
        foreach (KeyValuePair<string, string> pair in location_to_id_dict)
        {
            if (pair.Value == id)
            {
                return pair.Key;
            }
        }

        return null;
    }

    /**
    * @brief Retrieves the device associated with a given hardware ID.
    * @param id The hardware ID of the device.
    * @param device Reference to store the device if found.
    * @return True if the device is found, otherwise false.
    */
    private bool get_device_by_id(string id, ref SilSyncDevice device)
    {
        if (id != null)
        {
            dictionary_mutex.WaitOne();
            if (id_to_device_dict.ContainsKey(id))
            {
                device = id_to_device_dict[id];
                dictionary_mutex.ReleaseMutex();
                return true;
            }
        }
        dictionary_mutex.ReleaseMutex();
        return false;
    }

    /**
    * @brief Adds a new device to the dictionary.
    * @param server_address The server address from which the device communicates.
    * @param payload_in The payload containing information about the device.
    */
    private void add_device_to_dict(IPEndPoint server_address, payload_t payload_in)
    {
        SilSyncDevice device = new SilSyncDevice();

        device.id = payload_in.id.TrimEnd('_');
        device.client = new UdpClient(server_address.Address.ToString(), PORT);
        device.retransmit_delay = (byte)id_to_device_dict.Count;

        id_to_device_dict.Add(payload_in.id.TrimEnd('_'), device);
    }

    /**
    * @brief Updates the device dictionary with the given device.
    * @param device The device to update in the dictionary.
    */
    private void update_device_dict(SilSyncDevice device)
    {

        dictionary_mutex.WaitOne();
        id_to_device_dict[device.id] = device;
        dictionary_mutex.ReleaseMutex();

    }


    /**
    * @brief Assembles a quaternion from the provided payload.
    * @param payload_in The payload containing quaternion data.
    * @return Quaternion assembled from the payload data.
    */
    private Quaternion assemble_quaternion(payload_t payload_in)
    {
        float i = payload_in.quat_i * 180.0f / Mathf.PI;
        float j = payload_in.quat_j * 180.0f / Mathf.PI;
        float k = payload_in.quat_k * 180.0f / Mathf.PI;
        float r = payload_in.quat_real * 180.0f / Mathf.PI;

        return new Quaternion(i, j, k, r);
    }

    /**
    * @brief Assembles a packet for requesting a sample.
    * @param device The device for grabbing retransmit delay from.
    * @return The assembled sample packet.
    */
    private payload_t assemble_sample_packet(SilSyncDevice device)
    {
        return new payload_t { request = (byte)Requests.client_sample, response = (byte)Responses.no_resp, id = "client", quat_i = 0, quat_j = 0, quat_k = 0, quat_real = 0, accuracy = 0, time_stamp = 0, retransmit_delay = device.retransmit_delay };
    }

    /**
    * @brief Assembles a packet to broadcast for device discovery.
    * @param device The device for grabbing retransmit delay from.
    * @return The assembled discovery packet.
    */
    private payload_t assemble_discovery_packet()
    {
        return new payload_t { request = (byte)Requests.client_discovery, response = (byte)Responses.no_resp, id = "client", quat_i = 0, quat_j = 0, quat_k = 0, quat_real = 0, accuracy = 0, time_stamp = 0, retransmit_delay = 15 };
    }

    /**
    * @brief Assembles a packet for requesting imu calibration.
    * @param device The device for grabbing retransmit delay from.
    * @return The assembled calibration packet.
    */
    private payload_t assemble_calibration_packet(SilSyncDevice device)
    {
        return new payload_t { request = (byte)Requests.client_calibrate, response = (byte)Responses.no_resp, id = "client", quat_i = 0, quat_j = 0, quat_k = 0, quat_real = 0, accuracy = 0, time_stamp = 0, retransmit_delay = device.retransmit_delay };
    }

    /**
    * @brief Assembles a packet for affirming a response from server.
    * @param device The device for grabbing retransmit delay from.
    * @return The assembled affirmation packet.
    */
    private payload_t assemble_affirmative_packet(SilSyncDevice device)
    {
        return new payload_t { request = (byte)Requests.client_sample, response = (byte)Responses.client_affirmative, id = "client", quat_i = 0, quat_j = 0, quat_k = 0, quat_real = 0, accuracy = 0, time_stamp = 0, retransmit_delay = device.retransmit_delay };
    }

    /**
    * @brief Assembles a packet for requesting imu tare.
    * @param device The device for grabbing retransmit delay from.
    * @return The assembled calibration packet.
    */
    private payload_t assemble_tare_packet(SilSyncDevice device)
    {
        return new payload_t { request = (byte)Requests.client_tare, response = (byte)Responses.no_resp, id = "client", quat_i = 0, quat_j = 0, quat_k = 0, quat_real = 0, accuracy = 0, time_stamp = 0, retransmit_delay = device.retransmit_delay };
    }

    /**
    * @brief Receives a packet from bracelet and updates device with data from payload.
    * @param device Reference to the device from which to receive the packet.
    * @return The received payload.
    */
    private payload_t receive_packet(ref SilSyncDevice device)
    {
        IPEndPoint remoteEndPoint = new IPEndPoint(IPAddress.Any, 0);
        device.client.Client.ReceiveTimeout = (byte)(device.retransmit_delay + 1); //UDP receive timeout
        byte[] payload_in_bytes = device.client.Receive(ref remoteEndPoint); //receive packet (blocking)
        payload_t payload_in = BytesToStruct<payload_t>(payload_in_bytes);  //convert byte array to struct

        //update current device
        device.id = payload_in.id.TrimEnd('_'); //set id of current device
        device.imu_heading = assemble_quaternion(payload_in); //set new heading of current device
        device.frame_drop_count = 0;
        device.connection_state = true;

        return payload_in;

    }

    /**
    * @brief Sends a packet to the device.
    * @param device The device to which the packet will be sent.
    * @param payload_out The payload to be sent.
    */
    private void send_packet(SilSyncDevice device, payload_t payload_out)
    {
        //send request for data 
        byte[] pay_load_out_bytes = StructToBytes(payload_out);
        device.client.Send(pay_load_out_bytes, pay_load_out_bytes.Length);

    }

    /**
    * @brief Sends a packet to the device requesting a sample or operation (ex. tare, calibrate), and receives a response. 
    * @param device The device to which sending and receiving operations will be performed upon.
    * @param payload_out reference to payload_t struct containing the packet to send.
    * @param payload_in reference to payload_t struct where device response shall be saved. 
    * @return True if device responded within TIMEOUT_MS
    */
    private bool retransmit(ref SilSyncDevice device, payload_t payload_out, ref payload_t payload_in)
    {
        int socket_timeout = 0;
        int total_delay = 0;
        bool success = false;

        do
        {
            payload_out.retransmit_delay = device.retransmit_delay; //update payload out retransmit delay in case delay was borrowed
            send_packet(device, payload_out);

            try
            {
                payload_in = receive_packet(ref device);

                //sucessfully received packet:
                if (PRINT_LOGS)
                    print_packet(payload_in);

                increment_successful_transmissions();
                success = true;

            }
            catch (SocketException e)
            {
                //packet dropped
                Debug.Log(device.id + " dropped packet, retransmit delay: " + device.retransmit_delay);

                //update total delay time: 
                socket_timeout = device.retransmit_delay + 1;
                total_delay += socket_timeout;
            }

        } while (((total_delay + socket_timeout) <= TIMEOUT_MS) && !success); //loop until frame timeout exceeded to sucessful transfer


        if (!success)
        {
            device.frame_drop_count++;

            if (device.frame_drop_count > MAX_FRAMES_DROPPED)
                device.connection_state = false; //sensor is considered disconnected if it has dropped ~1 second worth of frames

            Debug.LogWarning(device.id + " dropped frame.");
        }

        update_device_dict(device); //update dictionary with device containing new info from receive_packet
        return success;
    }

    /**
    * @brief Requests a sample from IMU of specific device.
    * @param device The device to request a sample from.
    * @return True if the sample request was successfully handled, otherwise false.
    */
    private bool handle_sample_request(ref SilSyncDevice device)
    {
        payload_t payload_out = assemble_sample_packet(device);
        payload_t payload_in = assemble_sample_packet(device);

        return retransmit(ref device, payload_out, ref payload_in);

    }

    /**
    * @brief Requests calibration of IMU of specific device.
    * @param device The device for which calibration is requested.
    */
    private bool handle_calibration_request(ref SilSyncDevice device)
    {
        payload_t payload_out;
        payload_t payload_in;

        payload_out = assemble_calibration_packet(device);
        payload_in = assemble_calibration_packet(device);
        payload_in.response = (byte)Responses.no_resp;

        bool success = false;

        //send calibration requests and wait for success or failure response
        while (payload_in.response != (byte)Responses.server_success && payload_in.response != (byte)Responses.server_failure)
        {
            device.update_evt.WaitOne();
            device.update_evt.Reset();

            retransmit(ref device, payload_out, ref payload_in);
            data_ready_sem.Release();

            if (payload_in.response == (byte)Responses.server_success)
                success = true;

        }

        payload_out = assemble_affirmative_packet(device);
        payload_in.response = (byte)Responses.no_resp;

        //send affirmative packets responses and wait for sampling response
        while (payload_in.response != (byte)Responses.server_sampling)
        {
            device.update_evt.WaitOne();
            device.update_evt.Reset();

            retransmit(ref device, payload_out, ref payload_in);
            data_ready_sem.Release();

        }

        device.calibration_evt.Reset();

        return success;
    }

    /**
    * @brief Requests tare of IMU of specific device.
    * @param device The device for which tare is requested.
    */
    private bool handle_tare_request(ref SilSyncDevice device)
    {
        payload_t payload_out = assemble_tare_packet(device);
        payload_t payload_in = assemble_tare_packet(device);
        return retransmit(ref device, payload_out, ref payload_in);
    }

    /**
    * @brief Prints the content of a packet.
    * @param packet The packet to print.
    */
    private void print_packet(payload_t packet)
    {
        Debug.Log(TAG + "\n" +
            " | ID: " + packet.id.Trim('_') + "| Time Stamp: " + packet.time_stamp + " | Req: " + packet.request + '\n' +
            " | I: " + packet.quat_i.ToString("F2") + " | J: " + packet.quat_j.ToString("F2") + " | K: " + packet.quat_k.ToString("F2") +
            " | Real: " + packet.quat_real.ToString("F2") + " | Accuracy: " + packet.accuracy + " |/n" +
            " | VBat: " + packet.battery_voltage + "mv | PWRState: " + power_state_to_string(packet.power_state) + " |"
        );
    }

    /**
 * @brief Converts power_state enum byte to string for consolde debug statements.
 * @param power_status a byte containing the power state enum
 * @return the respective enum code converted to a string
 */
    private string power_state_to_string(byte power_state)
    {
        string power_state_str;

        switch ((PowerStates)power_state)
        {
            case PowerStates.USB_powered_charging:
                power_state_str = "USB Powered, Charging";
                break;

            case PowerStates.USB_powered_fully_charged:
                power_state_str = "USB Powered, Full Charge";
                break;

            case PowerStates.battery_powered:
                power_state_str = "Battery Powered";
                break;

            default:

                power_state_str = "undefined";

                break;
        }

        return power_state_str;
    }

    /**
    * @brief Updates the stable headings of all devices in the dictionary.
    */
    private void update_device_stable_headings()
    {
        dictionary_mutex.WaitOne();
        foreach (KeyValuePair<string, SilSyncDevice> pair in id_to_device_dict)
        {
            pair.Value.imu_heading_stable = pair.Value.imu_heading;
        }
        dictionary_mutex.ReleaseMutex();
    }

    /**
     * @brief Sets the update events for all devices in the dictionary telling them to initiate sending/recieving process.
     */
    private void set_update_events()
    {
        dictionary_mutex.WaitOne();
        foreach (KeyValuePair<string, SilSyncDevice> pair in id_to_device_dict)
        {
            pair.Value.update_evt.Set();
        }
        dictionary_mutex.ReleaseMutex();
    }

    /**
    * @brief Increments successful transmissions, called by transceiver threads upon successfuly send/receive cycle. 
    */
    private void increment_successful_transmissions()
    {
        successful_transmissions_mutex.WaitOne();
        successful_transmissions++;
        successful_transmissions_mutex.ReleaseMutex();
    }

    /**
   * @brief Resets successful transmissions, called by check_data thread after all transceiver threads have finished send/receive cycle (whether it is successful or not)
   */
    private void reset_successful_transmissions()
    {
        successful_transmissions_mutex.WaitOne();
        successful_transmissions = 0;
        successful_transmissions_mutex.ReleaseMutex();
    }

    /**
    * @brief Thread function for handling communication with a sensor, 1 of these threads exists per connected sensor.
    * @param arg The argument passed to the thread at launch, representing the body location of the sensor for which the thread corresponds.
    */
    private void transceiver_thread(object arg)
    {
        String location = (string)arg;
        SilSyncDevice device = new SilSyncDevice();
        bool success = false;

        while (!stop_threads_evt.WaitOne(0))
        {
            get_device_by_id(get_id_by_location(location), ref device); //grab the respective device for the body location passed at thread launch

            //only perform sending/receiving operations if the device is actually connected
            if (device != null)
            {
                if (!device.calibration_evt.WaitOne(0) && !device.tare_evt.WaitOne(0))
                {
                    device.update_evt.WaitOne(); //wait for signaling from check data watcher thread
                    device.update_evt.Reset(); //reset signal from check data watcher thread

                    //handle sample request
                    handle_sample_request(ref device);
                    data_ready_sem.Release(); //release data_ready sem to signal to check data thread that this transceiver thread has completed its send/receieve cycle


                }
                else if (device.calibration_evt.WaitOne(0))
                {
                    handle_calibration_request(ref device);



                }
                else if (device.tare_evt.WaitOne(0))
                {
                    device.update_evt.WaitOne();
                    device.update_evt.Reset();

                    //handle tare request
                    success = handle_tare_request(ref device);
                    data_ready_sem.Release();

                    //release tare event if successfully tared
                    if (success)
                        device.tare_evt.Reset();
                }

            }

        }

        if (get_device_by_id(get_id_by_location(location), ref device))
        {
            if (device.client != null)
                device.client.Close();
        }
    }

    /**
    * @brief Thread function for checking if data is ready from all devices.
    * @param arg n/a
    */
    private void check_data_thread(object arg)
    {
        long elapsed_time;
        long start_time;

        while (!stop_threads_evt.WaitOne(0))
        {
            start_time = DateTime.Now.Ticks;

            reset_successful_transmissions();
            set_update_events();

            for (int i = 0; i < id_to_device_dict.Count; i++)
                data_ready_sem.WaitOne();

            if (successful_transmissions == id_to_device_dict.Count)
            {
                update_device_stable_headings();
            }

            Thread.Sleep(1);

            //calculate how long to delay for to maintain ~15ms refresh rate
            elapsed_time = (long)Math.Round((double)(DateTime.Now.Ticks - start_time)/10000.0f);
            long delay = Math.Max(TIMEOUT_MS - elapsed_time, 0);
            Thread.Sleep((int)delay);

            data_ready_sem = new Semaphore(0, id_to_device_dict.Count); //reset data_ready_sem


        }

    }


    //convert struct to byte array 
    private byte[] StructToBytes<T>(T structure) where T : struct
    {
        int size = Marshal.SizeOf<T>();
        byte[] bytes = new byte[size];

        unsafe
        {
            fixed (byte* ptr = bytes)
            {
                Marshal.StructureToPtr(structure, (IntPtr)ptr, false);
            }
        }

        return bytes;
    }

    //convert byte array to struct using unsafe code
    private T BytesToStruct<T>(byte[] bytes) where T : struct
    {
        T structure;

        unsafe
        {
            fixed (byte* ptr = bytes)
            {
                structure = Marshal.PtrToStructure<T>((IntPtr)ptr);
            }
        }

        return structure;
    }
}
