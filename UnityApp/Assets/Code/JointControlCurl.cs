using UnityEngine;
using System;
using System.Net.Sockets;
using System.Threading;
using UnityEngine.UI;
using TMPro;

public class JointControlCurl : MonoBehaviour
{
    public Transform limb1;
    public Transform limb2;
    public Transform limb3;
    /*
    public Transform limb4;
    public Transform limb5;
    */

    public Calibration calbool;
    private IMUDataGrabber imu_data_stream = IMUDataGrabber.get_grabber(); 

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
    bool standingUp = false;
    bool curlTopOfRep = false;
    bool curlMiddleOfRep = false;
    bool curlBottomOfRep = false;
    bool curlStep1 = false;
    bool curlStep2 = false;
    bool curlStep3 = false;
    bool curlStep4 = false;
    bool curlStep5 = false;
    bool curlCycle = false;
    int repCounter = 0;

    void Start()
    {
        imu_data_stream.scan_for_devices(5);
    }

    void Update()
    {
        position1 = imu_data_stream.get_sensor_data(BodyLocation.leftWrist);
        position2 = imu_data_stream.get_sensor_data(BodyLocation.leftBicep);
        position3 = imu_data_stream.get_sensor_data(BodyLocation.chest);
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

        if(position3ProccessedWOffset.x > 340 || position3ProccessedWOffset.x < 10){
            if(position3ProccessedWOffset.z > 340 || position3ProccessedWOffset.z < 10){
                standingUp = true;
                //range for wrist sensor when grabbing bar
                /*if(position2ProccessedWOffset.x > 350 || position2ProccessedWOffset.x < 30){
                    if(position2ProccessedWOffset.z > 310 && position2ProccessedWOffset.z < 360){
                        Debug.Log("Ready to Bench");
                    }
                }
                */
                //range for bicep sensor when grabbing bar
                if(position1ProccessedWOffset.x > 320 || position1ProccessedWOffset.x < 30){
                    if(position2ProccessedWOffset.z > 100 && position2ProccessedWOffset.z < 150){
                        curlTopOfRep = true;
                    }
                    else curlTopOfRep = false;
                    if(position2ProccessedWOffset.z > 60 && position2ProccessedWOffset.z < 80){
                        curlMiddleOfRep = true;
                    }
                    else curlMiddleOfRep = false;
                    if(position2ProccessedWOffset.z < 40){
                        curlBottomOfRep = true;
                    }
                    else curlBottomOfRep = false;
                }
            }
        }
        else {
            standingUp = false;
        }

        if (standingUp){
            if (curlTopOfRep){
                curlCycle = true;
            }
        }
        else{
            curlCycle = false;
            curlStep1 = false;
            curlStep2 = false;
            curlStep3 = false;
            curlStep4 = false;
            curlStep5 = false;
        }

        if (curlCycle) {
            if (curlTopOfRep) {
                curlStep1 = true;
            }
            if (curlStep1) {
                if (curlMiddleOfRep) {
                    curlStep2 = true;
                }
            }
            else curlStep2 = false;
            if (curlStep2) {
                if (curlBottomOfRep) {
                    curlStep3 = true;
                }
            }
            else curlStep3 = false;
            if (curlStep3) {
                if (curlMiddleOfRep) {
                    curlStep4 = true;
                }
            }
            else curlStep4 = false;
            if (curlStep4) {
                if (curlTopOfRep) {
                    curlStep5 = true;
                }
            }
            else curlStep5 = false;
            if (curlStep1 && curlStep2 && curlStep3 && curlStep4 && curlStep5) {
                repCounter++;
                Debug.Log(repCounter);
                curlCycle = false;
                curlStep1 = false;
                curlStep2 = false;
                curlStep3 = false;
                curlStep4 = false;
                curlStep5 = false;
            }
        }

        //Debug.Log(position2ProccessedWOffset);
        
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
        }

        /*
        limb4.rotation = position4;
        limb5.rotation = position5;
        */
    }

    void OnApplicationQuit()
    {
     
    }


}