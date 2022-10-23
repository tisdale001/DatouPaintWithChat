# DatouPaintWithChat
## Group members:
Lucian Tisdale (myself)<br>
Aditya Sharma<br>
Ye 'Mary' Bai<br>
Chiemelie Ezeokeke<br>
Nathanial Ziegler<br>
## Background
This program was designed for the final project in Foundations of Software Engineering using C++ and SFML. It is a simple paint program with networking capability plus a chat feature. Multiple users could log on and paint together and/or chat. It highlighted an undo/redo function that allowed users to undo or redo entire brushstrokes. The Command pattern was used to accomplish this. For my part in the group: I provided the majority of the starter code, wrote the 'paintbrush' function, supervised merges to Github, and did pair-coding.<br>
## Features
* Draw with 8 different colors (black, white, red, green, blue, yellow, magenta, cyan)
* Variable paintbrush thickness (3 different sizes, but could be various sizes)
* Undo entire strokes of the paintbrush
* Redo entire strokes of the paintbrush
* Blank entire screen to one color (undo/redo this action)
* Unlimited undo/redo (up to physical RAM capacity)
* Free memory in the redo stack after drawing something new
* No memory leaks because all pointers are implemented with smart_ptr
* Also had chat feature (sending messages to other users)
* Networking capabilities (users could log on and paint together, undo/redo each other's work)
## How to Build
cd into the bin directory
```
cmake ..
make
```
![DatouPaint_1](https://user-images.githubusercontent.com/53150782/194772337-507bf929-c683-4714-9db3-f8b905270753.PNG)
