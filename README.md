# rotatingcube

This is a visual demo for Flipper Zero where you can rotate and move a cube in two axis.

Read more about Flipper Zero: https://docs.flipperzero.one/

![alt text](https://github.com/elch3rto/rotatingcube/blob/main/screencapture.jpeg "Screen capture from Flipper App")

---

## Controls

UP/DOWN/LEFT/RIGHT - Move or Rotate in the direction of the button

OK - Toggle between movement and rotation modes

BACK - Exit the application

## Installation
---

The instructions assume you know how to use the ```fbt``` CLI tool (from https://github.com/flipperdevices/flipperzero-firmware).
To build the application, clone the official flipperzero-firmware repo, clone the contents of this repo to [firmware folder]/applications_user/. and run: 
```
./fbt launch_app APPSRC=applications_user/rotatingcube
```
~~Alternatively, you can simply copy the *.fap* executable from the release folder into your Flipper's external memory (SD Card).~~
It is highly recommended to build the app from source, as API changes can prevent the *.fap* apps from launching.




