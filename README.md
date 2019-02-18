# UMC v0.101
Universal Motion Controller
It will turn your phone into motion controller which can be use under Unreal or Unity3D project - no platform dependent.
Solution allows to add multiply phones/UMC to one project so it can be used for both hands or for multiplayer app.
Main principle of this project is to make it open source and free - allow everyone from indie dev to AAA studios use it and develop.

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/YOUTUBE_VIDEO_ID_HERE/0.jpg)](https://www.youtube.com/watch?v=YOUTUBE_VIDEO_ID_HERE)

## Requirements
For UMC to work you need:
* Unreal or Unity3D project (can PIE)
* WiFi network with broadcasting enabled and port 2492 opened 
* [Android phone](https://developers.google.com/ar/discover/supported-devices) which supports [ARCore](https://developers.google.com/ar/)

## Setup
**Unreal**<br> 
Get files from *UMC/src/Unreal* and put it under *"Plugins/UMC"* in your project root or Unreal engine directly. In editor you can use UMCActor - it has an array of Actor which transform will be control. Take a look into example.

**Unity3D**<br>  
For Unity3D you have to just copy *UMCBehavior.cs* to your project. In Inspector you can set *ControllerGOs* and *MotionMultiply*

**Android**<br> 
You can grab apk from *apk* folder or build from source. Copy to phone and install. Current version has **no** Google Play setup.

## Examples
**Unreal**<br> 
To make Unreal example work, you need to put files from *UMC/src/Unreal* and put it under *"Plugins/UMC"*
Example itself has BP_UMCActor blueprint and example actor blueprint.

**Unity3D**<br> 
![alt text](https://i.imgur.com/yV2eN0W.png)

Example is fairly simple - grab and put 'denga' elements on scene.
There is no need to make additional setup.

## Personal request
If possible, please don't change 2492 port number - it's special number for me.

## TODO
- Create IOS version
- Make communication  more efficient - get rid off string

## Changelog
- 1.01
  - Fixed Unreal transform
