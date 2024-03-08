using UnityEngine;
using System;
using System.Net.Sockets;
using System.Threading;
using UnityEngine.UI;
using TMPro;

public class JointControl : MonoBehaviour
{
    public Transform limb1;
    public Transform limb2;
    public Transform limb3;
    /*
    public Transform limb4;
    public Transform limb5;
    */

    public Calibration calbool;
    private IMUDataGrabber imu_data_stream = new IMUDataGrabber(); 

    private Thread rx_data_thread;
    private ManualResetEvent stop_rx_data_thread_evt = new ManualResetEvent(false); // Event to signal the thread to stop 

    public TextMeshProUGUI sensor1Text;  
    public TextMeshProUGUI sensor2Text;
    public TextMeshProUGUI sensor3Text; 
    //public TextMeshProUGUI sensor4Text; 
    //public TextMeshProUGUI sensor5Text;  

    private Quaternion position1 = new Quaternion(1, 1, 1, 1);
    private Quaternion position2 = new Quaternion(1, 1, 1, 1);
    private Quaternion position3 = new Quaternion(1, 1, 1, 1);
    //private Quaternion position4;
    //private Quaternion position5;

    private Vector3 position1PreProccessed;
    private Vector3 position2PreProccessed;
    private Vector3 position3PreProccessed;

    private Quaternion position1proccessed;
    private Quaternion position2proccessed;
    private Quaternion position3proccessed;

    // Offset for calibrating model
    Quaternion SholderOffset  = new Quaternion(1, 1, 1, 1);
    Quaternion ForearmOffset  = new Quaternion(1, 1, 1, 1);
    Quaternion HipOffset  = new Quaternion(1, 1, 1, 1);

    Quaternion cal1Quat;
    Quaternion cal2Quat;
    Quaternion cal3Quat;

    void Start()
    {
        rx_data_thread = new Thread(ReceiveData);
        rx_data_thread.Start();
    }

    void Update()
    {
        /*
        position1proccessed = new Quaternion(-(position1.y - ySholderOffset), -(position1.x - xSholderOffset), -(position1.z - zSholderOffset), (position1.w));
        position2proccessed = new Quaternion(-(position2.y - yForearmOffset), -(position2.x - xForearmOffset), -(position2.z - zForearmOffset), (position2.w));
        position3proccessed = new Quaternion((position3.y), -(position3.y), (position3.z), position3.w);
        */
        position1proccessed = new Quaternion(-(position1.y), (position1.x), (position1.z), (position1.w));
        position2proccessed = new Quaternion(-(position2.y), (position2.x), (position2.z), (position2.w));
        position3proccessed = new Quaternion(-(position3.y), (position3.x), (position3.z), (position3.w));
        
        limb1.transform.rotation = SholderOffset * position1proccessed;
        limb2.transform.rotation = ForearmOffset * position2proccessed;
        limb3.transform.rotation = HipOffset * position3proccessed;

        sensor1Text.color = imu_data_stream.get_sensor_connection_text(0);
        sensor2Text.color = imu_data_stream.get_sensor_connection_text(1);
        sensor3Text.color = imu_data_stream.get_sensor_connection_text(2);

        if (calbool.calibrationButton) 
        {
            Vector3 p1 = new Vector3(90, 0, -90);
            Vector3 p2 = new Vector3(90, 0, -90);
            Vector3 p3 = new Vector3(0 ,180, 0);
            cal1Quat = Quaternion.Euler(p1);
            cal2Quat = Quaternion.Euler(p2);
            cal3Quat = Quaternion.Euler(p3);

            SholderOffset = cal1Quat * Quaternion.Inverse(position1proccessed);
            ForearmOffset = cal2Quat * Quaternion.Inverse(position2proccessed);
            HipOffset = cal3Quat * Quaternion.Inverse(position3proccessed);

            calbool.calibrationButton = false;
            //Debug.Log(xSholderOffset);
        }

        /*
        limb4.rotation = position4;
        limb5.rotation = position5;
        */
    }

    void OnApplicationQuit()
    {
        stop_rx_data_thread_evt.Set(); 
    }

    void ReceiveData()
    {

        while (!stop_rx_data_thread_evt.WaitOne(0))
        {
            try
            {
                position1 = imu_data_stream.get_sensor_data(0);
                position2 = imu_data_stream.get_sensor_data(1);
                position3 = imu_data_stream.get_sensor_data(2);
            }
            catch (Exception e)
            {
                Debug.LogWarning("Error: " + e.Message);
            }
        
        }
     
    }

}