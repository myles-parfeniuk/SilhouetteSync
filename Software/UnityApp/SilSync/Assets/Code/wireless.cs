using UnityEngine;
using System;
using System.Net.Sockets;
using System.Threading;
using System.Runtime.InteropServices;
using UnityEngine.UI;
using TMPro;

public class IMUController : MonoBehaviour
{
    //public static IMUController instance;
    [System.Serializable]
    public struct Payload
    {
        public byte id;
        public float x_heading;
        public float y_heading;
        public float z_heading;
        public byte accuracy;
    }
    public Transform limb1;
    public Transform limb2;
    /*
    public Transform limb3;
    public Transform limb4;
    public Transform limb5;
    */

    public TcpClient imuTcpClient1;
    public TcpClient imuTcpClient2;
    private NetworkStream imuNetworkStream1;
    private NetworkStream imuNetworkStream2;
    private Thread imuThread;
    private bool isRunning = true;

    private Quaternion position1;
    private Quaternion position2;
    /*
    private Quaternion position3;
    private Quaternion position4;
    private Quaternion position5;
    

    void Awake() {
        if(instance != null && instance != this) {
            DestroyImmediate(gameObject);
            return;
        }
        instance = this;
        DontDestroyOnLoad(gameObject);
    }
    */

    void Start()
    {
        imuThread = new Thread(ReceiveData);
        imuThread.Start();
    }

    void Update()
    {
        limb1.transform.rotation = position1;
        limb2.transform.rotation = position2;
        /*
        limb3.rotation = position1;
        limb4.rotation = position1;
        limb5.rotation = position1;
        */
    }

    void OnApplicationQuit()
    {
        isRunning = false;

        if (imuNetworkStream1 != null)
            imuNetworkStream1.Close();

        if (imuTcpClient1 != null)
            imuTcpClient1.Close();

        if (imuNetworkStream2 != null)
            imuNetworkStream2.Close();

        if (imuTcpClient2 != null)
            imuTcpClient2.Close();
    }

    void ReceiveData()
    {
        try
        {
            imuTcpClient1 = new TcpClient("192.168.187.50", 49160);
            imuNetworkStream1 = imuTcpClient1.GetStream();
            imuTcpClient2 = new TcpClient("192.168.187.65", 49160);
            imuNetworkStream2 = imuTcpClient2.GetStream();

            //long prevTime = 0;

            while (isRunning)
            {
                Payload payloadOut = new Payload { id = 0, x_heading = 0 };

                // Convert struct to byte array
                byte[] payloadBytes = StructToBytes(payloadOut);

                imuNetworkStream1.Write(payloadBytes, 0, payloadBytes.Length);
                Debug.Log("Sent " + payloadBytes.Length + " bytes");

                byte[] buffer = new byte[Marshal.SizeOf<Payload>()]; // Use Marshal.SizeOf to get the size of the struct
                int bytesRead = imuNetworkStream1.Read(buffer, 0, buffer.Length);

                // Convert byte array to struct
                Payload payloadIn = BytesToStruct<Payload>(buffer);
                float x = payloadIn.x_heading  * 180 / Mathf.PI;
                float y =  payloadIn.y_heading * 180 / Mathf.PI;
                float z = payloadIn.z_heading  * 180 / Mathf.PI;
                
                Debug.Log("Received id=" + payloadIn.id + ", X_heading=" + x.ToString("F4") +
                        ", Y_heading=" + y.ToString("F4") + ", Z_Heading=" + z.ToString("F4") +
                         ", Accuracy=" + payloadIn.accuracy);

                position1 = Quaternion.Euler(x, y, z);

                payloadOut = new Payload { id = 0, x_heading = 0 };

                // Convert struct to byte array
                payloadBytes = StructToBytes(payloadOut);

                imuNetworkStream2.Write(payloadBytes, 0, payloadBytes.Length);
                Debug.Log("Sent " + payloadBytes.Length + " bytes");

                buffer = new byte[Marshal.SizeOf<Payload>()]; // Use Marshal.SizeOf to get the size of the struct
                bytesRead = imuNetworkStream2.Read(buffer, 0, buffer.Length);

                // Convert byte array to struct
                payloadIn = BytesToStruct<Payload>(buffer);
                x = payloadIn.x_heading  * 180 / Mathf.PI;
                y =  payloadIn.y_heading * 180 / Mathf.PI;
                z = payloadIn.z_heading  * 180 / Mathf.PI;
                
                Debug.Log("Received id=" + payloadIn.id + ", X_heading=" + x.ToString("F4") +
                        ", Y_heading=" + y.ToString("F4") + ", Z_Heading=" + z.ToString("F4") +
                         ", Accuracy=" + payloadIn.accuracy);

                position2 = Quaternion.Euler(x, y, z);

                //long currentTime = DateTimeOffset.Now.ToUnixTimeMilliseconds();
                //Debug.Log("Received id=" + payloadIn.id + ", X_heading=" + payloadIn.x_heading.ToString("F4") +
                //         ", Y_heading=" + payloadIn.y_heading.ToString("F4") + ", Z_Heading=" + payloadIn.z_heading.ToString("F4") +
                //          ", Accuracy=" + payloadIn.accuracy + ", TIMESTAMP: " + ((currentTime - prevTime) / 1000.0).ToString("F4"));

                //prevTime = currentTime;

                // Add a delay to avoid flooding the server with requests
                //Thread.Sleep(10);
            }
        }
        catch (Exception e)
        {
            Debug.Log("Error: " + e.Message);
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