using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class RecalSensors : MonoBehaviour
{
    public bool sensorCalibrationLeftWrist = false;
    public bool sensorCalibrationLeftBicep = false;
    public bool sensorCalibrationChest = false;
    public bool sensorCalibrationRightWrist = false;
    public bool sensorCalibrationRightBicep = false;

    public void calibrationButtonPressedLeftWrist()
    {
        sensorCalibrationLeftWrist = true;
    }

    public void calibrationButtonPressedLeftBicep()
    {
        sensorCalibrationLeftBicep = true;
    }

    public void calibrationButtonPressedChest()
    {
        sensorCalibrationChest = true;
    }

    public void calibrationButtonPressedRightWrist()
    {
        sensorCalibrationRightWrist = true;
    }

    public void calibrationButtonPressedRightBicep()
    {
        sensorCalibrationRightBicep = true;
    }
}
