using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using UnityEngine;

public class SilSyncDevice
{
    public ulong id;
    public UdpClient client;
    public bool connection_state;
    public int disconnection_count;
    public byte retransmit_delay;
    public Quaternion imu_heading_stable;
    public Quaternion imu_heading;
    public ManualResetEvent update_evt; //event to transceiver thread to ask for a sample packet
    public ManualResetEvent calibration_evt; //event to signal transceiver thread to ask for calibration procedure
    public ManualResetEvent tare_evt; //event to signal tranceiver thread to ask for tare procedure
    public Thread transceiver_thread_hdl; 

    public SilSyncDevice()
    {
        id = 0; 
        client = null;
        connection_state = false;
        disconnection_count = 0;
        retransmit_delay = 0;
        imu_heading_stable = new Quaternion(0, 0, 0, 1);
        imu_heading = new Quaternion(0, 0, 0, 1);
        update_evt = new ManualResetEvent(false);
        calibration_evt = new ManualResetEvent(false);
        tare_evt = new ManualResetEvent(false);
        transceiver_thread_hdl = null;
    }
}
