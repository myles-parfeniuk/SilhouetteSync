using UnityEngine;
using System;
using System.Net.Sockets;
using System.Threading;
using UnityEngine.UI;
using TMPro;

public class JointControlBench : MonoBehaviour
{
    public Transform limb1;
    public Transform limb2;
    public Transform limb3;

    [SerializeField] private AudioClip highElbows;
    [SerializeField] private AudioClip oneCount;
    [SerializeField] private AudioClip twoCount;
    [SerializeField] private AudioClip threeCount;
    [SerializeField] private AudioClip fourCount;
    [SerializeField] private AudioClip fiveCount;
    [SerializeField] private AudioClip sixCount;
    [SerializeField] private AudioClip sevenCount;
    [SerializeField] private AudioClip eightCount;
    [SerializeField] private AudioClip nineCount;
    [SerializeField] private AudioClip tenCount;
    [SerializeField] private AudioClip elevenCount;
    [SerializeField] private AudioClip twelveCount;
    [SerializeField] private AudioClip thirteenCount;
    [SerializeField] private AudioClip fourteenCount;
    [SerializeField] private AudioClip fifteenCount;
    [SerializeField] private AudioClip sixteenCount;
    [SerializeField] private AudioClip seventeenCount;
    [SerializeField] private AudioClip eighteenCount;
    [SerializeField] private AudioClip nineteenCount;
    [SerializeField] private AudioClip twentyCount;
    [SerializeField] private AudioClip calibrationCompleteSound;
    [SerializeField] private AudioClip halfRep;

    private AudioSource audioSource;

    /*
    public Transform limb4;
    public Transform limb5;
    */

    public Calibration calbool;
    public RecalSensors leftWrist;
    public RecalSensors leftBicep;
    public RecalSensors chest;
    public RecalSensors rightBicep;
    public RecalSensors rightWrist;
    private IMUDataGrabber imu_data_stream = IMUDataGrabber.get_grabber(); 

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
    private Quaternion position1proccessedWithOffset;
    private Quaternion position2proccessedWithOffset;
    private Quaternion position3proccessedWithOffset;
    private Vector3 position1ProccessedWOffset;
    private Vector3 position2ProccessedWOffset;
    private Vector3 position3ProccessedWOffset;

    // Offset for calibrating model
    Quaternion SholderOffset  = new Quaternion(1, 1, 1, 1);
    Quaternion ForearmOffset  = new Quaternion(1, 1, 1, 1);
    Quaternion HipOffset  = new Quaternion(1, 1, 1, 1);

    Quaternion cal1Quat;
    Quaternion cal2Quat;
    Quaternion cal3Quat;
    bool layingDown = false;
    bool benchTopOfRep = false;
    bool benchMiddleOfRep = false;
    bool benchBottomOfRep = false;
    bool benchStep1 = false;
    bool benchStep2 = false;
    bool benchStep3 = false;
    bool benchStep4 = false;
    bool benchStep5 = false;
    bool benchStep6 = false;
    bool benchCycle = false;
    int repCounter = 0;

    void Start()
    {
        rx_data_thread = new Thread(ReceiveData);
        rx_data_thread.Start();
        audioSource = GetComponent<AudioSource>();
    }

    void Update()
    {
        /*
        position1proccessed = new Quaternion(-(position1.y - ySholderOffset), -(position1.x - xSholderOffset), -(position1.z - zSholderOffset), (position1.w));
        position2proccessed = new Quaternion(-(position2.y - yForearmOffset), -(position2.x - xForearmOffset), -(position2.z - zForearmOffset), (position2.w));
        position3proccessed = new Quaternion((position3.y), -(position3.y), (position3.z), position3.w);
        */
        position1proccessed = new Quaternion((position1.y), -(position1.x), (position1.z), (position1.w));
        position2proccessed = new Quaternion((position2.y), -(position2.x), (position2.z), (position2.w));
        position3proccessed = new Quaternion((position3.y), (position3.x), -(position3.z), (position3.w));

        position1proccessedWithOffset = SholderOffset * position1proccessed;
        position2proccessedWithOffset = ForearmOffset * position2proccessed;
        position3proccessedWithOffset = HipOffset * position3proccessed;

        position1ProccessedWOffset = position1proccessedWithOffset.eulerAngles;
        position2ProccessedWOffset = position2proccessedWithOffset.eulerAngles;
        position3ProccessedWOffset = position3proccessedWithOffset.eulerAngles;

        if(position3ProccessedWOffset.x > 270 && position3ProccessedWOffset.x < 300){
            layingDown = true;
            //range for wrist sensor when grabbing bar
            /*if(position2ProccessedWOffset.x > 350 || position2ProccessedWOffset.x < 30){
                if(position2ProccessedWOffset.z > 310 && position2ProccessedWOffset.z < 360){
                    Debug.Log("Ready to Bench");
                }
            }
            */
            //range for bicep sensor when grabbing bar
            if(position1ProccessedWOffset.x > 350 || position1ProccessedWOffset.x < 30){
                if(position1ProccessedWOffset.z > 0 && position1ProccessedWOffset.z < 20){
                    benchTopOfRep = true;
                }
                else benchTopOfRep = false;
                if(position1ProccessedWOffset.z > 40 && position1ProccessedWOffset.z < 50){
                    benchMiddleOfRep = true;
                }
                else benchMiddleOfRep = false;
                if(position1ProccessedWOffset.z > 60){
                    benchBottomOfRep = true;
                }
                else benchBottomOfRep = false;
            }
        }
        else {
            layingDown = false;
        }

        if (layingDown){
            if (benchTopOfRep){
                benchCycle = true;
            }
        }
        else{
            benchCycle = false;
            benchStep1 = false;
            benchStep2 = false;
            benchStep3 = false;
            benchStep4 = false;
            benchStep5 = false;
        }

        if (benchCycle) {
            if (benchTopOfRep) {
                benchStep1 = true;
            }
            if (benchStep1) {
                if (benchMiddleOfRep) {
                    benchStep2 = true;
                }
            }
            else benchStep2 = false;
            if (benchStep2) {
                if (benchBottomOfRep) {
                    benchStep3 = true;
                }
                if (benchTopOfRep) {
                    benchStep6 = true;
                }
            }
            else {
                benchStep3 = false;
                benchStep6 = false;
            }
            if (benchStep3) {
                if (benchMiddleOfRep) {
                    benchStep4 = true;
                }
            }
            else benchStep4 = false;
            if (benchStep4) {
                if (benchTopOfRep) {
                    benchStep5 = true;
                }
            }
            else benchStep5 = false;
            if (benchStep1 && benchStep2 && benchStep3 && benchStep4 && benchStep5) {
                repCounter++;
                Debug.Log(repCounter);
                if(repCounter == 1) {
                    audioSource.clip = oneCount;
                    audioSource.Play();
                }
                if(repCounter == 2) {
                    audioSource.clip = twoCount;
                    audioSource.Play();
                }
                if(repCounter == 3) {
                    audioSource.clip = threeCount;
                    audioSource.Play();
                }
                if(repCounter == 4) {
                    audioSource.clip = fourCount;
                    audioSource.Play();
                }
                if(repCounter == 5) {
                    audioSource.clip = fiveCount;
                    audioSource.Play();
                }
                if(repCounter == 6) {
                    audioSource.clip = sixCount;
                    audioSource.Play();
                }
                if(repCounter == 7) {
                    audioSource.clip = sevenCount;
                    audioSource.Play();
                }
                if(repCounter == 8) {
                    audioSource.clip = eightCount;
                    audioSource.Play();
                }
                if(repCounter == 9) {
                    audioSource.clip = nineCount;
                    audioSource.Play();
                }
                if(repCounter == 10) {
                    audioSource.clip = tenCount;
                    audioSource.Play();
                }
                if(repCounter == 11) {
                    audioSource.clip = elevenCount;
                    audioSource.Play();
                }
                if(repCounter == 12) {
                    audioSource.clip = twelveCount;
                    audioSource.Play();
                }
                if(repCounter == 13) {
                    audioSource.clip = thirteenCount;
                    audioSource.Play();
                }
                if(repCounter == 14) {
                    audioSource.clip = fourteenCount;
                    audioSource.Play();
                }
                if(repCounter == 15) {
                    audioSource.clip = fifteenCount;
                    audioSource.Play();
                }
                if(repCounter == 16) {
                    audioSource.clip = sixteenCount;
                    audioSource.Play();
                }
                if(repCounter == 17) {
                    audioSource.clip = seventeenCount;
                    audioSource.Play();
                }
                if(repCounter == 18) {
                    audioSource.clip = eighteenCount;
                    audioSource.Play();
                }
                if(repCounter == 19) {
                    audioSource.clip = nineteenCount;
                    audioSource.Play();
                }
                if(repCounter == 20) {
                    audioSource.clip = twentyCount;
                    audioSource.Play();
                }
                benchCycle = false;
                benchStep1 = false;
                benchStep2 = false;
                benchStep3 = false;
                benchStep4 = false;
                benchStep5 = false;
            }
            if (benchStep1 && benchStep2 && benchStep6) {

                audioSource.clip = halfRep;
                audioSource.Play();

                benchCycle = false;
                benchStep1 = false;
                benchStep2 = false;
                benchStep3 = false;
                benchStep4 = false;
                benchStep5 = false;
                benchStep6 = false;
            }
        }

        //Debug.Log(position1ProccessedWOffset);
        
        limb1.transform.rotation = position1proccessedWithOffset;
        limb2.transform.rotation = position2proccessedWithOffset;
        limb3.transform.rotation = position3proccessedWithOffset;

        sensor1Text.color = imu_data_stream.get_sensor_connection_text(BodyLocation.leftWrist);
        sensor2Text.color = imu_data_stream.get_sensor_connection_text(BodyLocation.leftBicep);
        sensor3Text.color = imu_data_stream.get_sensor_connection_text(BodyLocation.chest);

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

            audioSource.clip = calibrationCompleteSound;
            audioSource.Play();
        }

        if (leftWrist.sensorCalibrationLeftWrist) 
        {
            imu_data_stream.calibrate_sensor(BodyLocation.leftWrist);
            leftWrist.sensorCalibrationLeftWrist = false;
            Debug.Log("1");
        }

        if (leftBicep.sensorCalibrationLeftBicep) 
        {
            imu_data_stream.calibrate_sensor(BodyLocation.leftBicep);
            leftBicep.sensorCalibrationLeftBicep = false;
            Debug.Log("2");
        }

        if (chest.sensorCalibrationChest) 
        {
            imu_data_stream.calibrate_sensor(BodyLocation.chest);
            chest.sensorCalibrationChest = false;
            Debug.Log("3");
        }

        if (rightWrist.sensorCalibrationRightWrist) 
        {
            imu_data_stream.tare_sensor(BodyLocation.chest); 
            // INSERT CALIBRATION CODE HERE
            rightWrist.sensorCalibrationRightWrist = false;
            Debug.Log("4");
        }

        if (rightBicep.sensorCalibrationRightBicep) 
        {
            // INSERT CALIBRATION CODE HERE
            rightBicep.sensorCalibrationRightBicep = false;
            Debug.Log("5");
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
        imu_data_stream.scan_for_devices(3);

        while (!stop_rx_data_thread_evt.WaitOne(0))
        {
            try
            {
                position1 = imu_data_stream.get_sensor_data(BodyLocation.leftWrist);
                position2 = imu_data_stream.get_sensor_data(BodyLocation.leftBicep);
                position3 = imu_data_stream.get_sensor_data(BodyLocation.chest);
            }
            catch (Exception e)
            {
                Debug.LogWarning("Error: " + e.Message);
            }

          
            Thread.Sleep(18);

        }
     
    }

}