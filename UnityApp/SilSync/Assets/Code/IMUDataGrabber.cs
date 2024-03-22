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
    [System.Serializable]
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

    public const int DISCOVERY_RETRY_COUNT = 10; ///< Number of retries for device discovery.
    private const int MAX_DISCONNECTION_COUNT = 10; ///<Maximum count for disconnections allowed.
    private const int TIMEOUT_MS = 15; ///< Timeout value in milliseconds.
    private const int PORT = 49160; ///< Port number for communication.
    private const bool PRINT_LOGS = false; ///< Indicates whether logs should be printed or not.
    private const string TAG = "IMUDataGrabber: "; ///< Tag for identifying log messages.

    private static IMUDataGrabber instance; ///< Singleton instance of the IMUDataGrabber.
    private Thread check_data_thread_hdl; ///< Thread handle for checking data.
    private ManualResetEvent stop_threads_evt = new ManualResetEvent(false); ///< Event to signal all transceiver threads to stop.
    private Dictionary<string, SilSyncDevice> id_to_device_dict = new Dictionary<string, SilSyncDevice>(); ///< Dictionary mapping device IDs to SilSyncDevice objects.
    private Dictionary<string, string> location_to_id_dict = new Dictionary<string, string>(); ///< Dictionary mapping locations to device IDs.
    private Semaphore data_ready_sem; ///< Semaphore for signaling data readiness.
    private readonly Mutex dictionary_mutex = new Mutex(); ///< Mutex for ensuring thread safety in dictionary operations.


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
        location_to_id_dict.Add("chest", "samaritan-blender");
        location_to_id_dict.Add("leftBicep", "juicy-toaster");
        location_to_id_dict.Add("leftWrist", "paris-flexible");
        location_to_id_dict.Add("rightBicep", "gorilla-birdy");
        location_to_id_dict.Add("rightWrist", "imagination-wonderland");
        location_to_id_dict.Add("leftThigh", "heuristic-assault");
        location_to_id_dict.Add("leftCalf", "corp-burger");
        location_to_id_dict.Add("leftFoot", "dream-opachki");
        location_to_id_dict.Add("rightThigh", "heuristic-assault");
        location_to_id_dict.Add("righCalf", "bobik-distasteful");
    }

    /**
    * @brief Scans for devices and saves responding device addresses for transceiver threads.
    * @param max_devices The maximum number of devices to scan local netowrk for.
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

            //broadcast discovery request packet
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
                    if(location != null)
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
            check_data_thread_hdl = new Thread(check_data_thread);
            check_data_thread_hdl.Start();

            Debug.Log(TAG + "Discovery Complete IDs Found: ");
            foreach (KeyValuePair<string, SilSyncDevice> pair in id_to_device_dict)
            {
                Debug.Log(TAG + "ID: " + pair.Key);
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

        stop_threads_evt.Reset();

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
        if(location_to_id_dict.ContainsKey(location_one) && location_to_id_dict.ContainsKey(location_two))
        {
            dictionary_mutex.WaitOne();
            string temp = location_to_id_dict[location_one];
            location_to_id_dict[location_one] = location_to_id_dict[location_two];
            location_to_id_dict[location_two] = temp;
            dictionary_mutex.ReleaseMutex(); 

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

        if(id != null)
        {
            get_device_by_id(id, ref device);

            if(device != null)
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

        if(location_to_id_dict.ContainsKey(location))
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
        dictionary_mutex.WaitOne();
        foreach (KeyValuePair<string, string> pair in location_to_id_dict)
        {
            if (pair.Value == id)
            {
                dictionary_mutex.ReleaseMutex();
                return pair.Key;
            }
        }
        dictionary_mutex.ReleaseMutex();

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
        if(id != null)
        {
            if (id_to_device_dict.ContainsKey(id))
            {
                device = id_to_device_dict[id];

                return true;
            }
        }
     
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
        device.connection_state = false;
        device.disconnection_count = 0;
        device.imu_heading = new Quaternion(0, 0, 0, 1);

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
        return new payload_t { request = (byte)Requests.client_discovery, response = (byte)Responses.no_resp, id = "client", quat_i = 0, quat_j = 0, quat_k = 0, quat_real = 0, accuracy = 0, time_stamp = 0, retransmit_delay = 15};
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
        device.client.Client.ReceiveTimeout = TIMEOUT_MS; //UDP receive timeout
        byte[] payload_in_bytes = device.client.Receive(ref remoteEndPoint); //receive packet (blocking)
        payload_t payload_in = BytesToStruct<payload_t>(payload_in_bytes);  //convert byte array to struct

        //update current device
        device.connection_state = true; //sensor is connected if data is received within TIMEOUT_MS
        device.disconnection_count = 0; //reset disconnection count after successful read
        device.retransmit_delay = (byte)(id_to_device_dict.Count + 1);
        device.id = payload_in.id.TrimEnd('_'); //set id of current device
        device.imu_heading = assemble_quaternion(payload_in); //set new heading of current device

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
        device.update_evt.WaitOne();
        device.update_evt.Reset();
        device.client.Send(pay_load_out_bytes, pay_load_out_bytes.Length);

    }

    /**
    * @brief Requests a sample from IMU of specific device.
    * @param device The device to request a sample from.
    * @return True if the sample request was successfully handled, otherwise false.
    */
    private bool handle_sample_request(SilSyncDevice device)
    {
        payload_t payload_out = assemble_sample_packet(device);
        payload_t payload_in;

        send_packet(device, payload_out);

        try
        {
            payload_in = receive_packet(ref device);

            if (PRINT_LOGS)
                print_packet(payload_in);

            update_device_dict(device);

            return true; 

        }
        catch (SocketException e)
        {

            if (e is SocketException se && se.SocketErrorCode == SocketError.TimedOut)
                Debug.Log(TAG + "Receive operation timed out.");

            if (device.disconnection_count >= MAX_DISCONNECTION_COUNT)
                device.connection_state = false; //sensor is disconnected if no data received after max disconnection count amount of tries
            else
                device.disconnection_count++;

            return false;
        }

    }

    /**
    * @brief Requests calibration of IMU of specific device.
    * @param device The device for which calibration is requested.
    */
    private void handle_calibration_request(SilSyncDevice device)
    {
        payload_t payload_out;
        payload_t payload_in;

        payload_out = assemble_calibration_packet(device);
        payload_in.response = (byte)Responses.no_resp;

        //send calibration requests and wait for success or failure response
        while (payload_in.response != (byte)Responses.server_success && payload_in.response != (byte)Responses.server_failure)
        {
            send_packet(device, payload_out);

            try
            {
                payload_in = receive_packet(ref device);

                if (PRINT_LOGS)
                    print_packet(payload_in);

                update_device_dict(device);
                data_ready_sem.Release();

            }
            catch (SocketException e)
            {
                Debug.Log(TAG + "Calibration receive operation timed out.");
            }

        }

        payload_out = assemble_affirmative_packet(device);
        payload_in.response = (byte)Responses.no_resp;

        //send affirmative packets responses and wait for sampling response
        while (payload_in.response != (byte)Responses.server_sampling)
        {
            send_packet(device, payload_out);

            try
            {
                payload_in = receive_packet(ref device);

                if (PRINT_LOGS)
                    print_packet(payload_in);

                update_device_dict(device);
                data_ready_sem.Release();

            }
            catch (SocketException e)
            {
                Debug.Log(TAG + "Calibration receive operation timed out.");
            }

        }

        device.calibration_evt.Reset();
    }

    /**
    * @brief Requests tare of IMU of specific device.
    * @param device The device for which tare is requested.
    */
    private void handle_tare_request(SilSyncDevice device)
    {
        payload_t payload_out = assemble_tare_packet(device);
        payload_t payload_in;

        bool failure = true;

        while(failure)
        {
            send_packet(device, payload_out);

            try
            {
                payload_in = receive_packet(ref device);

                if (PRINT_LOGS)
                    print_packet(payload_in);

                update_device_dict(device);

                failure = false; 

            }
            catch (SocketException e)
            {

                if (e is SocketException se && se.SocketErrorCode == SocketError.TimedOut)
                    Debug.Log(TAG + "Receive operation timed out.");

                if (device.disconnection_count >= MAX_DISCONNECTION_COUNT)
                    device.connection_state = false; //sensor is disconnected if no data received after max disconnection count amount of tries
                else
                    device.disconnection_count++;

            }
        }

        device.tare_evt.Reset(); 

    }

    /**
    * @brief Prints the content of a packet.
    * @param packet The packet to print.
    */
    private void print_packet(payload_t packet)
    {
        Debug.Log(TAG + 
            " | ID: " + packet.id.Trim('_') + "| Time Stamp: " + packet.time_stamp + " | Req: " + packet.request + '\n' +
            " | I: " + packet.quat_i.ToString("F4") + " | J: " + packet.quat_j.ToString("F4") + " | K: " + packet.quat_k.ToString("F4") +
            " | Real: " + packet.quat_real.ToString("F4") + " | Accuracy: " + packet.accuracy + " |"
        );
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
    * @brief Thread function for handling communication with a sensor, 1 of these threads exists per connected sensor.
    * @param arg The argument passed to the thread at launch, representing the body location of the sensor for which the thread corresponds.
    */
    private void transceiver_thread(object arg)
    {
        String location = (string)arg;
        SilSyncDevice device = new SilSyncDevice();

        while (!stop_threads_evt.WaitOne(0))
        {
            get_device_by_id(get_id_by_location(location), ref device);

            if (device != null)
            {
                if (!device.calibration_evt.WaitOne(0) && !device.tare_evt.WaitOne(0))
                {
                    //handle sample request
                    if (handle_sample_request(device))
                        data_ready_sem.Release();
                }
                else if(device.calibration_evt.WaitOne(0))
                {
                    handle_calibration_request(device);

                }
                else if (device.tare_evt.WaitOne(0))
                {
                    handle_tare_request(device);
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
        bool all_devices_updated;

        while (!stop_threads_evt.WaitOne(0))
        {
            all_devices_updated = true;

            set_update_events();

            for (int i = 0; i < id_to_device_dict.Count; i++)
            {
                if (!data_ready_sem.WaitOne(TIMEOUT_MS))
                {
                    all_devices_updated = false;
                    break;
                }
            }

            data_ready_sem = new Semaphore(0, id_to_device_dict.Count); //reset data_ready_sem

            if (all_devices_updated)
            {
                update_device_stable_headings();
            }

            Thread.Sleep(5);
        }

    }


    // Convert struct to byte array using unsafe code
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

    // Convert byte array to struct using unsafe code
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
