using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DengaGrabber : MonoBehaviour
{
    public UMCBehavior UMC = null;
    public GameObject Grabber = null;

    private Vector3 GrabbRot = new Vector3();
    private bool IsGrabbing = false;
    
	void Update ()
    {
        if(Grabber != null)
        {
            Bounds GOBounds = gameObject.GetComponent<BoxCollider>().bounds;
            Bounds GrabberBounds = Grabber.GetComponent<BoxCollider>().bounds;
            if(IsGrabbing == false && GOBounds.Intersects(GrabberBounds) && UMC.GetTouches() > 1)
            {
                IsGrabbing = true;
                gameObject.GetComponent<BoxCollider>().enabled = false;
                GrabbRot = gameObject.transform.rotation.eulerAngles;
            }

            if(IsGrabbing && UMC.GetTouches() <= 1)
            {
                IsGrabbing = false;
                gameObject.GetComponent<BoxCollider>().enabled = true;
            }
        }        

        if (IsGrabbing)
        {
            Vector3 NewPosition = UMC.ControllerGOs[0].transform.position;
            Quaternion NewRotation = Quaternion.Euler(UMC.ControllerGOs[0].transform.rotation.eulerAngles + GrabbRot);
            gameObject.transform.position = NewPosition;
            gameObject.transform.rotation = Quaternion.Slerp(gameObject.transform.rotation, NewRotation, 0.5f);
        }
	}
}
