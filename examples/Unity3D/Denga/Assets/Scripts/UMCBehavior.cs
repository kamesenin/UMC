using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Linq;
using System.Text;

public class UMCBehavior : MonoBehaviour
{
    private enum MessageTypeEnum
    {
        Location,
        Rotation
    };

    private enum PlatformEnum
    {
        None,
        PC,
        Android
    };

    public List<GameObject> ControllerGOs;
    public float MotionMultiply = 10.0f;

    private Dictionary<string, float> AliveControllers = new Dictionary<string, float>();
    private Dictionary<string, Vector3> ControllersLocation = new Dictionary<string, Vector3>();
    private Dictionary<string, Quaternion> ControllersRotation = new Dictionary<string, Quaternion>();

    private float MaximumIdleTime = 5.0f;
    private float CurrentTime;
    private int CurrentTouchesCount = 0;
    private Dictionary<string, Vector3> Offsets = new Dictionary<string, Vector3>();

    private PlatformEnum Platform = PlatformEnum.None;

    private int Port = 2492;

    private UdpClient Client;
    private Thread ReceiveThread;
    
    private string PlatformStr;
    private string LocationStr;
    private string RotationStr;
    private string IDStr = "ID";
    private string TouchesStr = "Touches";

    private IPEndPoint SenderIP;
    private IPEndPoint ReciverIP;

    private void Start ()
    {
        Client = new UdpClient(Port);

        ReceiveThread = new Thread(
        new ThreadStart(ReceiveData));
        ReceiveThread.IsBackground = true;
        ReceiveThread.Start();

        PlatformStr = Platform.ToString();
        LocationStr = MessageTypeEnum.Location.ToString();
        RotationStr = MessageTypeEnum.Rotation.ToString();

        SenderIP = new IPEndPoint(IPAddress.Broadcast, Port);
        ReciverIP = new IPEndPoint(IPAddress.Any, 0);
        Platform = PlatformEnum.PC;
    }

    public int GetTouches()
    {
        return CurrentTouchesCount;
    }

    private void Update()
    {
        if (Client != null)
        {
            string TestMessage = Platform.ToString();
            byte[] Data = Encoding.UTF8.GetBytes(TestMessage);
            Client.Send(Data, Data.Length, SenderIP);
        }

        if (AliveControllers.Count > 0)
        {
            int Index = 0;
            List<string> IDs = new List<string>(AliveControllers.Keys);
            foreach (string ID in IDs)
            {
                Vector3 PositionTo = Offsets.Keys.Contains(ID) ? ControllersLocation[ID] - Offsets[ID] : ControllersLocation[ID];
                ControllerGOs[Index].transform.position = Vector3.Slerp(ControllerGOs[Index].transform.position, PositionTo, 0.25f);
                ControllerGOs[Index].transform.rotation = Quaternion.Slerp(ControllerGOs[Index].transform.rotation, ControllersRotation[ID], 0.5f);
                ++Index;
            }

            CurrentTime = Time.time;
            CheckConnectedControllers();
        }

        if (CurrentTouchesCount >= 5)
        {
            List<string> IDs = new List<string>(AliveControllers.Keys);
            foreach (string ID in IDs)
            {
                if (Offsets.Keys.Contains(ID))
                {
                    Offsets[ID] = ControllersLocation[ID];
                }
                else
                {
                    Offsets.Add(ID, ControllersLocation[ID]);
                }
            }
        }
    }

    private void OnDisable()
    {
        if (ReceiveThread != null && ReceiveThread.IsAlive)
        {
            ReceiveThread.Abort();
        }
        if (Client != null) { Client.Close(); }
    }

    private void ReceiveData()
    {
        while (true)
        {
            try
            {
                byte[] Data = Client.Receive(ref ReciverIP);
                string MessageFrom = Encoding.UTF8.GetString(Data);
                string[] splitString = MessageFrom.Split(new string[] { "#" }, StringSplitOptions.None);

                if (splitString[0] != PlatformStr)
                {
                    string ID = splitString[2];
                    if (splitString[1] == IDStr)
                    {
                        if (AliveControllers.Keys.Contains(ID) == false)
                        {
                            AliveControllers.Add(ID, CurrentTime);
                            ControllersLocation.Add(ID, Vector3.zero);
                            ControllersRotation.Add(ID, Quaternion.identity);
                        }
                        else
                        {
                            AliveControllers[ID] = CurrentTime;
                        }
                    }

                    if (splitString[3] == LocationStr)
                    {
                        string[] SplitLocationString = splitString[4].Split(new string[] { "," }, StringSplitOptions.None);
                        Vector3 LocationFromPhone = new Vector3(float.Parse(SplitLocationString[0]) * MotionMultiply, float.Parse(SplitLocationString[1]) * MotionMultiply, -float.Parse(SplitLocationString[2]) * MotionMultiply);
                        ControllersLocation[ID] = LocationFromPhone;
                    }

                    if (splitString[5] == RotationStr)
                    {
                        string[] SplitRotationString = splitString[6].Split(new string[] { "," }, StringSplitOptions.None);
                        Quaternion RotationFromPhone = new Quaternion(float.Parse(SplitRotationString[0]), float.Parse(SplitRotationString[1]), float.Parse(SplitRotationString[2]), float.Parse(SplitRotationString[3]));
                        ControllersRotation[ID] = RotationFromPhone;
                    }

                    if (splitString[7] == TouchesStr)
                    {
                        CurrentTouchesCount = int.Parse(splitString[8]);
                    }
                }
            }
            catch (Exception err)
            {
                print(err);
            }
        }
    }

    private void CheckConnectedControllers()
    {
        bool ShouldRemoveID = false;
        int ToRemoveIndex = 0;
        int CurrentIndex = 0;

        List<string> IDs = new List<string>(AliveControllers.Keys);
        foreach (string ID in IDs)
        {
            float LastTimeUpdated = Time.time - AliveControllers[ID];
            if (LastTimeUpdated > MaximumIdleTime)
            {
                ShouldRemoveID = true;
                break;
            }
            ++ToRemoveIndex;
        }

        if (ShouldRemoveID == false) { return; }

        foreach (string ID in IDs)
        {
            if (CurrentIndex == ToRemoveIndex)
            {
                AliveControllers.Remove(ID);
                ControllersLocation.Remove(ID);
                ControllersRotation.Remove(ID);
            }

            if (CurrentIndex > ToRemoveIndex)
            {
                AliveControllers.Remove(ID);               
                ControllersLocation.Remove(ID);            
                ControllersRotation.Remove(ID);    
            }
        }
    }
}
