using UnityEngine;
using System;
using System.Net.Sockets;
using System.Threading;
using System.Runtime.InteropServices;
using UnityEngine.UI;
using TMPro;

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
    public Transform limb1;
    public Transform limb2;
    public Transform limb3;
    /*
    public Transform limb4;
    public Transform limb5;
    */

    private bool sensor1State = false;
    private bool sensor2State = false;
    private bool sensor3State = false;
    //private bool sensor4State;
    //private bool sensor5State;

    public Calibration calbool;

    public TcpClient imuTcpClient1;
    public TcpClient imuTcpClient2;
    public TcpClient imuTcpClient3;
    //public TcpClient imuTcpClient4;
    //public TcpClient imuTcpClient5;
    private NetworkStream imuNetworkStream1;
    private NetworkStream imuNetworkStream2;
    private NetworkStream imuNetworkStream3;
    //private NetworkStream imuNetworkStream4;
    //private NetworkStream imuNetworkStream5;
    private Thread imuThread;
    private bool isRunning = true;
    public TextMeshProUGUI sensor1Text;  
    public TextMeshProUGUI sensor2Text;
    public TextMeshProUGUI sensor3Text; 
    //public TextMeshProUGUI sensor4Text; 
    //public TextMeshProUGUI sensor5Text;  
    private Quaternion position1;
    private Quaternion position2;
    private Quaternion position3;
    //private Quaternion position4;
    //private Quaternion position5;
    private Quaternion pos1cal = Quaternion.Euler(0, 0, 0);
    private Quaternion pos2cal = Quaternion.Euler(0, 0, 0);
    private Quaternion pos3cal = Quaternion.Euler(0, 0, 0);
    //private Quaternion pos4cal = Quaternion.Euler(0, 0, 0);
    //private Quaternion pos5cal = Quaternion.Euler(0, 0, 0);

    void Start()
    {
        imuThread = new Thread(ReceiveData);
        imuThread.Start();
        sensor1State = false;
        sensor2State = false;
    }

    void Update()
    {
        limb1.transform.rotation = position1;
        limb2.transform.rotation = position2;
        limb3.transform.rotation = position3;
        if (sensor1State)
        {
            sensor1Text.color = Color.green;
        }
        else
        {
            sensor1Text.color = Color.red;
        }
        if (sensor2State)
        {
            sensor2Text.color = Color.green;
        }
        else
        {
            sensor2Text.color = Color.red;
        }
        if (sensor3State)
        {
            sensor3Text.color = Color.green;
        }
        else
        {
            sensor3Text.color = Color.red;
        }
        if (calbool) 
        {
            pos1cal = position1;
            pos2cal = position2;
            pos3cal = position3;
        }
        /*
        limb4.rotation = position4;
        limb5.rotation = position5;
        */
    }

    void OnApplicationQuit()
    {
        isRunning = false;

        if (imuNetworkStream1 != null)
            imuNetworkStream1.Close();

        if (imuTcpClient1 != null)
        {
            imuTcpClient1.Close();
            sensor1State = false;
        }

        if (imuNetworkStream2 != null)
            imuNetworkStream2.Close();

        if (imuTcpClient2 != null)
        {
            imuTcpClient2.Close();
            sensor2State = false;
        }

        if (imuNetworkStream3 != null)
            imuNetworkStream3.Close();

        if (imuTcpClient3 != null)
        {
            imuTcpClient3.Close();
            sensor3State = false;
        }
    }

    void ReceiveData()
    {
        try
        {
            imuTcpClient1 = new TcpClient("192.168.168.50", 49160);
            imuNetworkStream1 = imuTcpClient1.GetStream();
            sensor1State = true;
            imuTcpClient2 = new TcpClient("192.168.168.248", 49160);
            imuNetworkStream2 = imuTcpClient2.GetStream();
            sensor2State = true;
            imuTcpClient3 = new TcpClient("192.168.168.", 49160);
            imuNetworkStream3 = imuTcpClient3.GetStream();
            sensor3State = true;

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

                //copy and paste from here on
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

                //copy and paste from here on
                payloadOut = new Payload { id = 0, x_heading = 0 };

                // Convert struct to byte array
                payloadBytes = StructToBytes(payloadOut);

                imuNetworkStream3.Write(payloadBytes, 0, payloadBytes.Length);
                Debug.Log("Sent " + payloadBytes.Length + " bytes");

                buffer = new byte[Marshal.SizeOf<Payload>()]; // Use Marshal.SizeOf to get the size of the struct
                bytesRead = imuNetworkStream3.Read(buffer, 0, buffer.Length);

                // Convert byte array to struct
                payloadIn = BytesToStruct<Payload>(buffer);
                x = payloadIn.x_heading  * 180 / Mathf.PI;
                y =  payloadIn.y_heading * 180 / Mathf.PI;
                z = payloadIn.z_heading  * 180 / Mathf.PI;
                
                Debug.Log("Received id=" + payloadIn.id + ", X_heading=" + x.ToString("F4") +
                        ", Y_heading=" + y.ToString("F4") + ", Z_Heading=" + z.ToString("F4") +
                         ", Accuracy=" + payloadIn.accuracy);

                position3 = Quaternion.Euler(x, y, z);
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