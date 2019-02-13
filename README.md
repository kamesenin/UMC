# UMC
Universal Motion Controller
It will turn your phone into motion controller which can be use under Unreal or Unity3D project - no platform dependent.
Solution allows to add multiply phones/UMC to one project so it can be used for both hands or for multiplayer app.
Main priciple of this project is to make it open source and free - allow everyone from indie dev to AAA studios use it and develop.

## Requirements
For UMC to work you need:
* Unreal or Unity3D project (can PIE)
* WiFi network with broadcasting enabled and port 2492 opened 
* [Android phone](https://developers.google.com/ar/discover/supported-devices) which supports [ARCore](https://developers.google.com/ar/)

## Setup
**Unreal** 


**Unity3D** 
For Unity3D you have to just copy *UMCBehavior.cs* to your project. In Inspector you can set **ControllerGOs** and **MotionMultiply**

## TODO
- Create IOS version
- Make comunication more efficient - get rid off string
