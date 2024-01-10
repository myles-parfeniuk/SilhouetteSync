using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Threading;
using UnityEngine.UI;

public class MyListener : MonoBehaviour
{
    Thread thread;
    public int connectionPort = 25001;
    TcpListener server;
    TcpClient client;
    bool running;

    [SerializeField]
    public Image panel;
    public Renderer sphereRenderer;
    public Color UnconnectedSphereColor = new Color(1f, 0f, 0f);
    public Color ConnectedSphereColor = new Color(0f, 1f, 0f);


    void Start()
    {
        //panel.color = UnconnectedSphereColor;
        // Receive on a separate thread so Unity doesn't freeze waiting for data
        ThreadStart ts = new ThreadStart(GetData);
        thread = new Thread(ts);
        thread.Start();
    }

    void GetData()
    {
        // Create the server
        server = new TcpListener(IPAddress.Any, connectionPort);
        server.Start();

        // Create a client to get the data stream
        client = server.AcceptTcpClient();

        // Start listening
        running = true;
        //panel.color = ConnectedSphereColor;
        while (running)
        {
            Connection();
        }
        server.Stop();
        //panel.color = UnconnectedSphereColor;
    }

    void Connection()
    {
        // Read data from the network stream
        NetworkStream nwStream = client.GetStream();
        byte[] buffer = new byte[client.ReceiveBufferSize];
        int bytesRead = nwStream.Read(buffer, 0, client.ReceiveBufferSize);

        // Decode the bytes into a string
        string dataReceived = Encoding.UTF8.GetString(buffer, 0, bytesRead);
        
        // Make sure we're not getting an empty string
        //dataReceived.Trim();
        if (dataReceived != null && dataReceived != "")
        {
            // Convert the received string of data to the format we are using
            position = ParseData(dataReceived);
            nwStream.Write(buffer, 0, bytesRead);
        }
    }

    // Use-case specific function, need to re-write this to interpret whatever data is being sent
    public static Quaternion ParseData(string dataString)
    {
        Debug.Log(dataString);
        // Remove the parentheses
        if (dataString.StartsWith("(") && dataString.EndsWith(")"))
        {
            dataString = dataString.Substring(1, dataString.Length - 2);
        }

        // Split the elements into an array
        string[] stringArray = dataString.Split(',');

        // Store as a quat
            float xtilt = float.Parse(stringArray[0]);
            float ytilt = float.Parse(stringArray[1]);
            float ztilt = float.Parse(stringArray[2]);

        Quaternion result = Quaternion.Euler(xtilt, ytilt, ztilt);

        return result;
    }

    // Position is the data being received in this example
    Quaternion position = Quaternion.Euler(0,0,0);

    void Update()
    {
        // Set this object's position in the scene according to the position received
        transform.rotation = position;
    }
}
