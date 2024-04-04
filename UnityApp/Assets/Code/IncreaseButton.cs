using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;
 
public class ScoreScript : MonoBehaviour {
 
    public TextMeshProUGUI scoreText;
    private int score;
 
    // Use this for initialization
    void Start()
    {
        score = 0;
        updateScoreUI();
    }
    public void addScore(int xAmount)
    {
        score = int.Parse(scoreText.text);
        score += xAmount;
        updateScoreUI();
    }
 
    public void ReduceScore(int yAmount)
    {
        score = int.Parse(scoreText.text);
        score -= yAmount;
        updateScoreUI();
    }
 
    void updateScoreUI()
    {
        scoreText.text = score.ToString();
    }
}