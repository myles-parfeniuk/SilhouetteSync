using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class SettingsCode : MonoBehaviour
{
    [SerializeField] GameObject WorkoutPage;
    [SerializeField] GameObject SettingsMenu;
    [SerializeField] GameObject setTarget;
    [SerializeField] GameObject sensorsStatus;
    [SerializeField] GameObject firmwareCalibration;
    [SerializeField] GameObject sensorCalibration;
    public void settingsButton() 
    {
        WorkoutPage.SetActive(false);
        SettingsMenu.SetActive(true);
    }
    public void closeSettingsButton() 
    {
        WorkoutPage.SetActive(true);
        SettingsMenu.SetActive(false);
    }
    public void changeTarget() 
    {
        SettingsMenu.SetActive(false);
        setTarget.SetActive(true);
        sensorCalibration.SetActive(false);
    }
        public void doneSettingTarget() 
    {
        WorkoutPage.SetActive(true);
        setTarget.SetActive(false);
    }
    public void calibrateSensors() 
    {
        sensorsStatus.SetActive(false);
        sensorCalibration.SetActive(true);
        SettingsMenu.SetActive(false);
        setTarget.SetActive(false);
    }
    public void doneCalibrating() 
    {
        SettingsMenu.SetActive(false);
        setTarget.SetActive(true);
        sensorCalibration.SetActive(false);
    }
        public void checkSensorStatus() 
    {
        SettingsMenu.SetActive(false);
        setTarget.SetActive(false);
        sensorsStatus.SetActive(true);
        sensorCalibration.SetActive(false);
    }
        public void recalibrateFirmware() 
    {
        firmwareCalibration.SetActive(true);
        SettingsMenu.SetActive(false);
    }
        public void doneRecalibrateFirmware() 
    {
        firmwareCalibration.SetActive(false);
        SettingsMenu.SetActive(true);
    }
}