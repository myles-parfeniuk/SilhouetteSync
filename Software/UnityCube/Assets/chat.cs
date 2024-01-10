using UnityEngine;
using System;
using System.Net.Sockets;
using System.Threading;
using System.Runtime.InteropServices;

public class IMUController : MonoBehaviour
{
    [System.Serializable]
    public struct Payload
    {
        public byte id;
        public float x_heading;
        public float y_heading;
        public float z_heading;
        public byte accuracy;
    }

    private TcpClient imuTcpClient;
    private NetworkStream imuNetworkStream;
    private Thread imuThread;
    private bool isRunning = true;

    private Quaternion position;


    void Start()
    {
        imuThread = new Thread(ReceiveData);
        imuThread.Start();
    }

    void Update()
    {
        transform.rotation = position;
    }

    void OnApplicationQuit()
    {
        isRunning = false;

        if (imuNetworkStream != null)
            imuNetworkStream.Close();

        if (imuTcpClient != null)
            imuTcpClient.Close();
    }

    void ReceiveData()
    {
        try
        {
            imuTcpClient = new TcpClient("192.168.253.65", 49160);
            imuNetworkStream = imuTcpClient.GetStream();

            long prevTime = 0;

            while (isRunning)
            {
                Payload payloadOut = new Payload { id = 0, x_heading = 0 };

                // Convert struct to byte array
                byte[] payloadBytes = StructToBytes(payloadOut);

                imuNetworkStream.Write(payloadBytes, 0, payloadBytes.Length);
                Debug.Log("Sent " + payloadBytes.Length + " bytes");

                byte[] buffer = new byte[Marshal.SizeOf<Payload>()]; // Use Marshal.SizeOf to get the size of the struct
                int bytesRead = imuNetworkStream.Read(buffer, 0, buffer.Length);

                // Convert byte array to struct
                Payload payloadIn = BytesToStruct<Payload>(buffer);
                float x = payloadIn.x_heading  * 180 / Mathf.PI;
                float y =  payloadIn.y_heading * 180 / Mathf.PI;
                float z = payloadIn.z_heading  * 180 / Mathf.PI;
                
                Debug.Log("Received id=" + payloadIn.id + ", X_heading=" + x.ToString("F4") +
                        ", Y_heading=" + y.ToString("F4") + ", Z_Heading=" + z.ToString("F4") +
                         ", Accuracy=" + payloadIn.accuracy);

                position = Quaternion.Euler(x, y, z);

                long currentTime = DateTimeOffset.Now.ToUnixTimeMilliseconds();
                //Debug.Log("Received id=" + payloadIn.id + ", X_heading=" + payloadIn.x_heading.ToString("F4") +
                //         ", Y_heading=" + payloadIn.y_heading.ToString("F4") + ", Z_Heading=" + payloadIn.z_heading.ToString("F4") +
                //          ", Accuracy=" + payloadIn.accuracy + ", TIMESTAMP: " + ((currentTime - prevTime) / 1000.0).ToString("F4"));

                prevTime = currentTime;

                // Add a delay to avoid flooding the server with requests
                //Thread.Sleep(10);
            }
        }
        catch (Exception e)
        {
            Debug.LogError("Error: " + e.Message);
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