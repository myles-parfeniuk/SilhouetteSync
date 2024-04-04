using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;
 
public class RepButton : MonoBehaviour {
 
    public TextMeshProUGUI repText;
    public int repTarget;
 
    // Use this for initialization
    void Start()
    {
        repTarget = 0;
        updateScoreUI();
    }
    public void addScore(int xAmount)
    {
        repTarget = int.Parse(repText.text);
        repTarget += xAmount;
        updateScoreUI();
    }
 
    public void ReduceScore(int yAmount)
    {
        repTarget = int.Parse(repText.text);
        repTarget -= yAmount;
        updateScoreUI();
    }
 
    void updateScoreUI()
    {
        repText.text = repTarget.ToString();
    }
}