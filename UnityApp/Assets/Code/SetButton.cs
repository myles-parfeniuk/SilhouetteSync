using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;
 
public class SetButton : MonoBehaviour {
 
    public TextMeshProUGUI setText;
    public int setTarget;
 
    // Use this for initialization
    void Start()
    {
        setTarget = 0;
        updateScoreUI();
    }
    public void addScore(int xAmount)
    {
        setTarget = int.Parse(setText.text);
        setTarget += xAmount;
        updateScoreUI();
    }
 
    public void ReduceScore(int yAmount)
    {
        setTarget = int.Parse(setText.text);
        setTarget -= yAmount;
        updateScoreUI();
    }
 
    void updateScoreUI()
    {
        setText.text = setTarget.ToString();
    }
}