# 3D-Scene-Editor
project website: https://zilixie.github.io/3D-Scene-Editor/

A 3D scene editor implemented in C++ combined with OpenGL. It supports the functionalities of insert, tranlate, colorize, rotate, scale, animate, delete for triangle mesh objects in .OFF file format and it allows users to export scene into .svg files.

## How To Use
>O: Start Translation mode.</br>
>P: Start Deletion mode.</br>
>U: Start Animation mode.</br>
>SPACE: Start/Stop the rotation of camera (by default the camera rotates arounf Y axis).</br>
><, >: Change color for selected object.</br>
>/: Change projection (perspective or orthogonal)＜/br＞
>[,]: change the shading method (vertex normal or face normal)</br>
>↑,↓,←,→: Move the camera on trackball up, down, left, right respectively by 2 degree.</br>
>+,-: Zoom in or out by 3%.</br>
>J,M: Rotate the selected Object by 2 degree clockwise or counter-clockwise around the Y axis.</br>
>H,N: Rotate the selected Object by 2 degree clockwise or counter-clockwise around the Z axis.</br>
>G,B: Rotate the selected Object by 2 degree clockwise or counter-clockwise around the X axis.</br>
>K,L: Scale the selected object up or down by 5% respectively.</br>
>W,S: Move the camera on Y axis.</br>
>E,D: Move the camera on X axis.</br>
>R,F: Move the camera on Z axis.</br>
>Z: Start the animation if the bezier curve for an object was constructed. </br>
>X: Take a screenshot and export scene to .svg file. </br>
>Q: Quit the program.</br>

## Operations
The following section will introduce all the operations supported by this application.

## Object Insert
>Instruction:</br>
>press '1' on keyboard will insert a unit cube;</br>
>press '2' on keyboard will insert a bunny;</br>
>press '3' on keyboard will insert a bumpy cube;</br>

## Object Control
>Instruction:</br>
>press 'O' on keyboard;</br>
>click on an object;</br>
>release the mouse;</br>
>press J,M to rotation on Y axis;</br>
>press H,N to rotation on Z axis;</br>
>press G,B to rotation on X axis;</br>
>press K,L to scale;</br>
>Rotate, Scale, Translate

## Camera & View Control
>Instruction:
>press ↑,↓,←,→ to move position of camera (in a trackball);</br>
>scroll up or down to change the distance from camera to origin;</br>
>move camera

## Animation
>Instruction:</br>
>press 'U' on keyboard;</br>
>click on an object;</br>
>click on a place where you want to place the first control point;</br>
>click on a place where you want to place the second control point;</br>
>click on a place where you want to place the third control point;</br>
>click on a place where you want to place the forth control point;</br>
>press 'Z' to start or pause animation; </br>
>click on control point and don't release, drag the control point will edit the bezier curve; </br>
>click on other object to make animation for other object or see the control points if bezier curve has been contructed;


## Export SVG
>Instruction:
>press 'X' anytime when you want to capture the scene;

![image](https://github.com/zilixie/3D-Scene-Editor/blob/master/images/snap1.svg | width=48)
![image](https://github.com/zilixie/3D-Scene-Editor/blob/master/images/snap2.svg)
