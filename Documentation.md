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
4. I have integrated Imgui succesfully. Now I will try to get imGizmo to work.

5. I have the mouse visibility toggling using left ctrl. Bug: when the mouse is disabled I cannot move the Imgui UI. Also you can pan around the scene using movement keys which is fine I guess. There was a popping issue when toggling mouse visibility but I fixed it ( basically we were moving back to old position of some internal values which tracked mouse position ).

6. imgui handles i/o on it own and it recommends to use its interface to handle them. And right now we are overriding the input handling from imgui to handle it in our application code and therefore the imgui frame didnot move. I need to handle mouse inputs from imgui now... lots of changes needs to be done now.
https://github.com/ocornut/imgui/issues/4664
Solved: We need to first pass input values to imgui then you can check if imgui is using that input and then process the input in your application.

7. I think its time for imGizmo ?
8. We have imguizmo almost. Need to abstract the code but for some reason there is some error in cmakelist: I cannot include ImGuizmo.h in any other file other than main.cpp. Alright fixed this imguizmo also requires imgui to be included when used.
9. Gizmo is working. We can switch between translate, rotate, scale using 1,2, and 3 key. Although we are missing a dedicated input handling system and tracking which model is selected because right now 1,2 and 3 key would effect all the models. So maybe next thing is to handle multiple models or defining a input handling system.
10. Loading multiple models -- Loading and saving models to be loaded and thier related textures in a .yml file. Maybe later in future we want to read/write the binary data representing the object containing these data into a file directly for faster load times. First step is to render multiple models then we do the yml file thing.
11. Now that we can almost render multiple models. We need to tidy this: Only show gizmo when a model is selected and only for that selected model. Selection of model is not implemented right now. We can select the model from IMGUI UI basically create a simple outliner. And later worry about selecting and show the UI feedback which I think needs to first have a bounding box of sorts ( define something in code on CPU side which hugs the object and when the mouse is within this and click event occured we selected the object). I have decided to create an outliner class which will keep track of all the models that are there in the scene.
12. outliner is working.
13. Now CPU raypicking for object selection is also working. Just need to make it visually clear.
14. There is a bug with the mouse click where it is detected even when I leave the left click. fixed them.
15. implmented changing transformation from outliner as well
16. Next should be - Solving shader issues ( probably start PBR ), Implmenting Level so I can save the position of the objects. Then it should be loading animations.
17. PBR progress following opengl lighting tutorial: I need to send Material textures from mesh.cpp. Also load correct ones to disk. Update the shader program with pbr.frag. UPDATE: PBR basic is working with point lights.
18. Working with files. First try storing model in a custom file and try loading model instances using this file.

## How to start the application
1. you need to use cmake.
2. X(+ve right) Y(+ve up) Z(+ve outside of screen)


## TODOs
1. (Done)Gizmo to transform the loaded 3d model. This way we might not need frame function immidiatly -- Do it ASAP.
2. Loading Textures:
    1. Interpret from the Assimp data - not done.
    2. Load explicitly. Or Later we could create a PBR material - Done.
    3. Default color for when texture or vertex color is not provided - partially done I defaulted the vertex color to be red.
    4. Need to default frag color on a model if no texture is provided. Don't use vertex color as base color and instead provide a default color. This way a boolean useTexture would suffice - Done
2.1. Load animations
2.2 Collisions  
3. building binaries for my game; game engine will be used as a library I think
    1. Lua scripting language integration https://gamedev.stackexchange.com/questions/421/how-do-you-add-a-scripting-language-to-a-game

## Dealing with errors

1. Use RenderDoc.
2. Error  "no vertex shader bound at draw" means probably the type of the layout in vertex is wrong or one of the value is not passed.
3. How does Input system work: I have a InputHandler static class just get the static member and access the buttons that you want to know the state of.
4. To convert a point from world space to local space -> inverse(modelMatrix) * vec4(pointInWorldSpace, 1.0);
5. To convert a point from local space to world space -> modelMatrix * vec4(pointInLocalSpace);
6. returning after imgui want to capture means that your mouse is probably over a imgui. Note setting leftClick to false when imgui is using inputs helped a ton.
7. Avoid circular includes or else you will get compile errors like no overload function found that takes n arg.
8. ` error LNK2038: mismatch detected for 'RuntimeLibrary': value 'MTd_StaticDebug' doesn't match value 'MDd_DynamicDebug' in test.obj ( for lots obj against the test.obj ) ........and then at last I get fatal error LNK1169: one or more multiply defined symbols found.` -- The GlitterLib was being built with sources of deps whose libs were already being linked to it. So only the .cpp files for your projects are needed here rest comes from linker. Also make sure in build/cmakecache.txt -- `gtest_force_shared_crt:BOOL=ON`.
9. BoneIds and thier weights on each vertex is not being transmitted correctly to shader.
10. `redefiniton error due stb_image and stb_write_png libraries`. Create a .cpp file and include their implmentation macro then headers in that file. Its because they came with .cpp and .h files instead of objs/binaries but the .cpp file was not created so we do this. This is probably for making them system agonstic.
11. I was trying to move the main.cpp around so the correct way is to modify what it is called in project_sources file. Don't create another file decalartion with same identifier.
12. I used clion to refactor the code. Right now cpp extension doesnot have that kind of sophisticated support.
13. Wrote a little tasks.json to create cpp file from a hpp file


## How does Level file works
1. ModelType is saved in a custom format Reprensenting the model class (This is basically the serialized version of a model object). This is so that we can load the model instantly from this file since it has has all the data needed to render the model.
2. Loading the model itself saves the model in the custom format.
3. model textures -- they will need to be saved again -- we save them again during load -- saved them in different file (Yes for reusability purposes) ?
4. In Level file we create instances of a model whose data is read from the above mentioned model file. For a model instance we store the location of the model from where we can construct this instance + transformation of the model instance in the level. We store an array of model instances and thier transformation in the level file.
5. When you save a model in the engine you should probably show it in outliner so we can drag and drop to use it in the level.
6. Try implmenting the importing/loading/saving workflow in a model. This will be the same way you need to do in a level.
    1. Import a model works
    2. Save a model works
    3. Load a model doesnot work
        i. we need to send the textures back to GPU
        ii. save textures in another file done
        iii. load a model
        iv. updated saving model; also savig textureIds but the code is failing currently.
        v. Texture loading works
    4. Textures are bleeding between models when loaded using our custom model file
        i. I think we are setting up textureIds that are not set. Because all the texture ids are constant.
        ii. We can leave this issue since textureBleeding won't happen if all the material textures are provided.
        iii. First implment Level saving, Then we should add ability to edit the material texture so we can fix this issue manually.
    5. Saving Level
        i. make sure that the model files are saved first. If not save them. Note now the model is saved in a file format that can be read by our program.
        ii. store the modelFilePath in an array in the level.
        iii. When loading the level load those models.
        iv. What about the game loop that gets model referenced in LevelFile to render. I think this is why we need a map. And while saving the level file just save the names in an array.
        v. Saving LvlAs working. What else
            # Save the model locations. -- done
            # Save the camera position. -- done
        vi. create a scenery
    6. Loading AnimationType
        1. Updating our Data structure. Storeing animation data.
            ## Bone over time applies transformation to vertices in a mesh that it influences.
                i. Bone AnimationType and Bone influence of each vertices of a mesh is stored separatly.
                ii. Bone OffsetMatrix. Vertex moved in bone space.
            ## Inverse kinematics and Forward Kinematics.
    6.1. Character class. 
    6.11. Asset Browser.
        i. Hover bug: When mouse hover over Imgui UIs, UI elements are getting hovered even when mouse is hidden.
    6.12. Material Manager.
    6.2 Loading multiple characters. To manage and fix textures.
    6.21. Saving Animations, character, etc...
    6.3 Playing different animation based on player input.
    7. Physics

## How does the UI code works
    1. Global state tracks which UI window is open.
    2. The UI elements are reusable and modular.

## Testing
    1. Using GoogleTests: https://matgomes.com/integrate-google-test-into-cmake/