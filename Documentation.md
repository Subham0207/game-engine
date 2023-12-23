## Camera
Starting the application you are spawned with a camera that you can control.
There is no way to see what things are loaded into the scene. There is nothing controlling that at the moment.

## Progress
Loading a 3d model is possible with limitation that only obj format is supported currently. Which I need to change immidiately and update the whole thing essentially and have a clean abstraction.

**I should try to first load a fbx file instead since that is format I could be using most often.
** approach is delete the currently loading logic which the mesh and model files and try to write it again but consider loading a fbx file probably lots of reading to do from Assimp documentation and take help from other's code on github.
** deadline --  15 days - write now day 1

## How to start the application
1. you need to use cmake.
2. X Y Z



## Dealing with errors

1. Use RenderDoc.
2. Error  "no vertex shader bound at draw" means probably the type of the layout in vertex is wrong or one of the value is not passed.