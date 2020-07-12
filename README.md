# 3D-Scene-Editor
project website: https://zilixie.github.io/3D-Scene-Editor/

A 3D scene editor implemented in C++ combined with OpenGL. It supports the functionalities of insert, tranlate, colorize, rotate, scale, animate, delete for triangle mesh objects in .OFF file format and it allows users to export scene into .svg files.

## How To Use

O: Start Translation mode.
P: Start Deletion mode.
U: Start Animation mode.
SPACE: Start/Stop the rotation of camera (by default the camera rotates arounf Y axis).
<, >: Change color for selected object.
/: Change projection (perspective or orthogonal)
[,]: change the shading method (vertex normal or face normal)
↑,↓,←,→: Move the camera on trackball up, down, left, right respectively by 2 degree.
+,-: Zoom in or out by 3%.
J,M: Rotate the selected Object by 2 degree clockwise or counter-clockwise around the Y axis.
H,N: Rotate the selected Object by 2 degree clockwise or counter-clockwise around the Z axis.
G,B: Rotate the selected Object by 2 degree clockwise or counter-clockwise around the X axis.
K,L: Scale the selected object up or down by 5% respectively.
W,S: Move the camera on Y axis.
E,D: Move the camera on X axis.
R,F: Move the camera on Z axis.
Z: Start the animation if the bezier curve for an object was constructed. 
X: Take a screenshot and export scene to .svg file. 
Q: Quit the program.

## Operations
The following section will introduce all the operations supported by this application.

## Object Insert
Instruction:
press '1' on keyboard will insert a unit cube;
press '2' on keyboard will insert a bunny;
press '3' on keyboard will insert a bumpy cube;

## Object Control
Instruction:
press 'O' on keyboard;
click on an object;
release the mouse;
press J,M to rotation on Y axis;
press H,N to rotation on Z axis;
press G,B to rotation on X axis;
press K,L to scale;
Rotate, Scale, Translate

## Camera & View Control
Instruction:
press ↑,↓,←,→ to move position of camera (in a trackball);
scroll up or down to change the distance from camera to origin;
move camera

## Animation
Instruction:
press 'U' on keyboard;
click on an object;
click on a place where you want to place the first control point;
click on a place where you want to place the second control point;
click on a place where you want to place the third control point;
click on a place where you want to place the forth control point;
press 'Z' to start or pause animation; 
click on control point and don't release, drag the control point will edit the bezier curve; 
click on other object to make animation for other object or see the control points if bezier curve has been contructed;
one object

## Export SVG
Instruction:
press 'X' anytime when you want to capture the scene;
