## Camera
Starting the application you are spawned with a camera that you can control.
There is no way to see what things are loaded into the scene. There is nothing controlling that at the moment.

## Progress
Loading a 3d model is possible with limitation that only obj format is supported currently. Which I need to change immidiately and update the whole thing essentially and have a clean abstraction.

**I should try to first load a fbx file instead since that is format I could be using most often.
** approach is delete the currently loading logic which the mesh and model files and try to write it again but consider loading a fbx file probably lots of reading to do from Assimp documentation and take help from other's code on github.
** deadline --  15 days - write now day 1
1. I can load load FBX or any other file format I understand the code and how its done: 1 VAO is allocated to each 3Model.
I have given the model a default vertex color if none exists. Now we need to figure out how to load textures and if no texture is found go with vertex color and if no vertex color is found put a default vertex color.
2. Tried to create a frame model function so that the camera could frame a 3d model but didnot succeed. I think the camera is facing the -180deg but not sure.

3. I need to create a Gizmo - lets have a separate shader to handle this. Lets try out the package called ImGizmo ( it requries imGui as dependency which we need in future tbh )

## How to start the application
1. you need to use cmake.
2. X(+ve right) Y(+ve up) Z(+ve outside of screen)


## TODOs
1. Gizmo to transform the loaded 3d model. This way we might not need frame function immidiatly -- Do it ASAP.
2. Loading Textures:
    1. Interpret from the Assimp data - not done.
    2. Load explicitly. Or Later we could create a PBR material - Done.
    3. Default color for when texture or vertex color is not provided - partially done I defaulted the vertex color to be red.
    4. Need to default frag color on a model if no texture is provided. Don't use vertex color as base color and instead provide a default color. This way a boolean useTexture would suffice - Done

## Dealing with errors

1. Use RenderDoc.
2. Error  "no vertex shader bound at draw" means probably the type of the layout in vertex is wrong or one of the value is not passed.