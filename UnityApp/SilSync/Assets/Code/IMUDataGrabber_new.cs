using UnityEngine;
using System.Runtime.InteropServices;
using System;
using System.Threading;
using System.Net;
using System.Net.Sockets;

public class IMUDataGrabber
{
    [System.Serializable]
    public struct payload_t
    {
        public byte request;
        public byte response;
        public ulong id;
        public float quat_i;
        public float quat_j;
        public float quat_k;
        public float quat_real; 
        public byte accuracy; 
        public ulong time_stamp;
    }

    private UdpClient[] udp_clients;
    private bool[] sensor_connection_state;
    private Quaternion[] imu_headings;
    private int[] sensor_disconnection_count;
    private const int SENSOR_COUNT = 3;
    private const int MAX_DISCONNECTION_COUNT = 10;
    private const int TIMEOUT_MS = 18;
    private const int DELAY_MS = 5; 
    private const bool PRINT_LOGS = true; 

    private enum Requests
    {
        sample,
        tare,
        calibrate
    }

    private enum Responses
    {
        busy = 255,
        success = 254,
        failure = 253,
        affirmative = 252,
        none = 0
    } 


    public IMUDataGrabber()
    {
        udp_clients = new UdpClient[SENSOR_COUNT]
        {
            new UdpClient("172.20.10.6", 49160),
            new UdpClient("172.20.10.7", 49160),
            new UdpClient("172.20.10.5", 49160)
        };

        //create arrays for connection_state, disconnection count and imu headings
        sensor_connection_state = new bool[SENSOR_COUNT] 
        { 
            false, 
            false, 
            false
        };

        sensor_disconnection_count = new int[SENSOR_COUNT]
        {
            0,
            0,
            0
        };

        imu_headings = new Quaternion[SENSOR_COUNT] 
        { 
            new Quaternion(0, 0, 0, 1),
            new Quaternion(0, 0, 0, 1),
            new Quaternion(0, 0, 0, 1)
        };

    }

    public Quaternion get_sensor_data(int sensor_id)
    {
        payload_t payload_out = new payload_t { request = (byte)Requests.sample, response = (byte)Responses.none, id = 0,  quat_i = 0, quat_j = 0, quat_k = 0, quat_real = 0, accuracy = 0, time_stamp = 0};

        //send request for data (empty buffer)
        byte[] pay_load_out_bytes = StructToBytes(payload_out);
        udp_clients[sensor_id].Send(pay_load_out_bytes, pay_load_out_bytes.Length);

        IPEndPoint remoteEndPoint = new IPEndPoint(IPAddress.Any, 0);
        udp_clients[sensor_id].Client.ReceiveTimeout = TIMEOUT_MS; //UDP receive timeout

        try
        {
            byte[] payload_in_bytes = udp_clients[sensor_id].Receive(ref remoteEndPoint);
            sensor_connection_state[sensor_id] = true; //sensor is connected if data is received within TIMEOUT_MS
            sensor_disconnection_count[sensor_id] = 0; //reset disconnection count after successful read
            payload_t payload_in = BytesToStruct<payload_t>(payload_in_bytes);  // Convert byte array to struct

            float i = payload_in.quat_i * 180 / Mathf.PI;
            float j = payload_in.quat_j * 180 / Mathf.PI;
            float k = payload_in.quat_k * 180 / Mathf.PI;
            float r = payload_in.quat_real * 180 / Mathf.PI;

            if (PRINT_LOGS)
               Debug.Log(
                    " | ID: 0x" + payload_in.id.ToString("X") + "| Time Stamp: " + payload_in.time_stamp + " | Req: " + payload_in.request + '\n' +
                    " | I: " + payload_in.quat_i.ToString("F4") + " | J: " + payload_in.quat_j.ToString("F4") + " | K: " + payload_in.quat_k.ToString("F4") +
                    " | Real: " + payload_in.quat_real.ToString("F4") + " | Accuracy: " + payload_in.accuracy + " |"
                );

            //5ms delay
            Thread.Sleep(DELAY_MS);

            imu_headings[sensor_id] = new Quaternion(i, j, k, r);
            
            return imu_headings[sensor_id]; 

        }
        catch (SocketException e)
        {

            if (e is SocketException se && se.SocketErrorCode == SocketError.TimedOut)
                Debug.LogWarning("Receive operation timed out.");

            if (sensor_disconnection_count[sensor_id] >= MAX_DISCONNECTION_COUNT)
                sensor_connection_state[sensor_id] = false; //sensor is disconnected if no data received after max disconnection count amount of tries
            else
                sensor_disconnection_count[sensor_id]++;

            return imu_headings[sensor_id]; //return last read position 
        }

    }

    public bool get_sensor_connection_state(int sensor_id)
    {
        return sensor_connection_state[sensor_id];
    }

    public Color get_sensor_connection_text(int sensor_id)
    {
        if (get_sensor_connection_state(sensor_id))
        {
            return Color.green;
        }
        else
        {
            return Color.red;
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
