using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class Calibration : MonoBehaviour
{
    public bool calibrationButton = false;
    public TextMeshProUGUI calibrationText;
    private string defaultText = "Press the calibrate button and stand in a T-pose until calibration complete.";
    private string threeText = "3";
    private string twoText = "2";
    private string oneText = "1";
    private string doneText = "Done!";

    private void calibrationButtonPressed()
    {
        StartCoroutine(DelayAction(1));
    }

    IEnumerator DelayAction(float delayTime)
    {
        calibrationText.fontSize = 200;
        calibrationText.text = threeText;
        yield return new WaitForSeconds(delayTime);
        calibrationText.text = twoText;
        yield return new WaitForSeconds(delayTime);
        calibrationText.text = oneText;
        yield return new WaitForSeconds(delayTime);
        calibrationButton = true;
        calibrationText.text = doneText;
        yield return new WaitForSeconds(delayTime);
        calibrationText.fontSize = 90;
        calibrationText.text = defaultText;
    }
}
