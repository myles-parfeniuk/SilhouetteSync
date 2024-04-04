using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class ButtonCode : MonoBehaviour {
    public void NextPage(int sceneid) {
        SceneManager.LoadScene(sceneid);
    }
}