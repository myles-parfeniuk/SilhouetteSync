using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using UnityEngine;

public class SilSyncDevice
{
    public string id;///<Hardware ID of device
    public UdpClient client; ///<UDP client object responsible for communication with this device instance. 
    public bool connection_state;///<Connection state of the device.
    public int frame_drop_count;///<Amount of frames dropped by this device (failed to respond within 15ms)
    public byte retransmit_delay; ///<Respective retransmit delay of device.
    public Quaternion imu_heading_stable; ///<Stable IMU heading (only updates when successful send/read cycle has been completed for all devices connected)
    public Quaternion imu_heading; ///<Last read IMU heading, copied to imu_heading_stable upon successful read/write cycle completion for all devices connected. 
    public ManualResetEvent update_evt; ///<Event to transceiver thread to ask for a sample packet
    public ManualResetEvent calibration_evt; ///<Event to signal transceiver thread to ask for calibration procedure
    public ManualResetEvent tare_evt; ///<Event to signal tranceiver thread to ask for tare procedure
    public Thread transceiver_thread_hdl; ///<Handle of thread responsible for running client for this respective device. 

    public SilSyncDevice()
    {
        id = null;
        client = null;
        connection_state = false;
        frame_drop_count = 0;
        retransmit_delay = 0;
        imu_heading_stable = new Quaternion(0, 0, 0, 1);
        imu_heading = new Quaternion(0, 0, 0, 1);
        update_evt = new ManualResetEvent(false);
        calibration_evt = new ManualResetEvent(false);
        tare_evt = new ManualResetEvent(false);
        transceiver_thread_hdl = null;
    }
}
