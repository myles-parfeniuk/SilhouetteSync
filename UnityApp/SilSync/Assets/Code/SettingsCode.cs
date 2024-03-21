using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class SettingsCode : MonoBehaviour
{
    [SerializeField] GameObject settingsMenuButton;
    [SerializeField] GameObject SettingsMenu;
    [SerializeField] GameObject doneTargetSet;
    [SerializeField] GameObject sensorsConnected;
    [SerializeField] GameObject recalibrationMenu;
    [SerializeField] GameObject doneCalibration;
    public void settingsButton() 
    {
        settingsMenuButton.SetActive(false);
        SettingsMenu.SetActive(true);
        doneTargetSet.SetActive(false);
        sensorsConnected.SetActive(false);
        //doneCalibration.SetActive(false);
        //recalibrationMenu.SetActive(false);
    }
    public void closeButton() 
    {
        settingsMenuButton.SetActive(true);
        SettingsMenu.SetActive(false);
        doneTargetSet.SetActive(false);
        sensorsConnected.SetActive(false);
        //doneCalibration.SetActive(false);
        //recalibrationMenu.SetActive(false);
    }
    public void changeTarget() 
    {
        settingsMenuButton.SetActive(false);
        SettingsMenu.SetActive(false);
        doneTargetSet.SetActive(true);
        sensorsConnected.SetActive(false);
        doneCalibration.SetActive(false);
        //recalibrationMenu.SetActive(false);
    }
        public void doneSettingTarget() 
    {
        settingsMenuButton.SetActive(true);
        SettingsMenu.SetActive(false);
        doneTargetSet.SetActive(false);
        sensorsConnected.SetActive(false);
        doneCalibration.SetActive(false);
        //recalibrationMenu.SetActive(false);
    }
    public void doneSensorStatus() 
    {
        settingsMenuButton.SetActive(false);
        SettingsMenu.SetActive(false);
        doneTargetSet.SetActive(false);
        sensorsConnected.SetActive(false);
        doneCalibration.SetActive(true);
        //recalibrationMenu.SetActive(false);
    }
    public void doneCalibrating() 
    {
        settingsMenuButton.SetActive(false);
        SettingsMenu.SetActive(false);
        doneTargetSet.SetActive(true);
        sensorsConnected.SetActive(false);
        doneCalibration.SetActive(false);
        //recalibrationMenu.SetActive(false);
    }
        public void checkSensorStatus() 
    {
        settingsMenuButton.SetActive(false);
        SettingsMenu.SetActive(false);
        doneTargetSet.SetActive(false);
        sensorsConnected.SetActive(true);
        doneCalibration.SetActive(false);
        //recalibrationMenu.SetActive(false);
    }

        public void recalibrateSensors() 
    {
        recalibrationMenu.SetActive(true);
        settingsMenuButton.SetActive(false);
        SettingsMenu.SetActive(false);
        //doneTargetSet.SetActive(false);
        //sensorsConnected.SetActive(false);
        //doneCalibration.SetActive(false);
    }
        public void doneRecalibrateSensors() 
    {
        recalibrationMenu.SetActive(false);
        settingsMenuButton.SetActive(false);
        SettingsMenu.SetActive(true);
        //doneTargetSet.SetActive(false);
        //sensorsConnected.SetActive(false);
        //doneCalibration.SetActive(false);
    }
}
