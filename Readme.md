## Getting Started
Start by cloning this repository, making sure to pass the `--recursive` flag to grab all the dependencies. 
If you forgot, then you can `git submodule update --init` instead.

```bash
git clone --recursive https://github.com/Subham0207/game-engine/
cd Glitter
cd Build
```

# Integrating Boost library
1. download source code from https://www.boost.org/releases/latest/.
2. extract file and put in under ./Glitter/Vendor/
3. cd into boost directory. For more detail follow https://www.boost.org/doc/user-guide/getting-started.html or run following cmds
    `run booststrap.bat`
    then `b2.exe` to compile lib. And then
    `b2 install --prefix=../Boost_install`
4. Note the version of boost is what is referenced in cmakelist of your project. So if you downloaded a different version update that.

Now, run build from vs code or execute `.\run_debug.bat` to generate build files
Then make sure in build/cmakecache.txt to make -- `gtest_force_shared_crt:BOOL=ON`. Read More detail on this in Documention file of this project.

# Integrating Lua and sol2 libraries:
Lua is a scripting library. Sol2 simplifies the use of Lua.
1. Download binaries of Lua from https://luabinaries.sourceforge.net/
2. Place it under ./Glitter/Vendor/ and things should work
3. sol2 is being fetched by project cmakefile using fetch content.
4. the cmake file automatically copies the lua dll from lua directory into exe's directory. if it fails check directory names.

# Only if using VS-Code
Add this config to .vscode/settings.json. 
This is the limit the search so we ignore any dependent library code.
The last line of configurationprovider is so the intellisense is handled via cmake's state.
```
    "files.exclude": {
        "**/build/": true,
        "**/bin/": true,
        "**/Vendor/": true
    },
    "search.exclude": {
        "**/build/": true,
        "**/bin/": true,
        "**/Vendor/": true
    },
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
```

## Documentation

Functionality           | Library
----------------------- | ------------------------------------------
Mesh Loading            | [assimp](https://github.com/assimp/assimp)
Physics                 | [bullet](https://github.com/bulletphysics/bullet3)
Physics                 | [JoltPhysics](https://github.com/bulletphysics/bullet3)
UI                      | [ImGui](https://github.com/bulletphysics/bullet3)
Gizmo                   | [ImGuizmo](https://github.com/bulletphysics/bullet3)
OpenGL Function Loader  | [glad](https://github.com/Dav1dde/glad)
Windowing and Input     | [glfw](https://github.com/glfw/glfw)
OpenGL Mathematics      | [glm](https://github.com/g-truc/glm)
Texture Loading         | [stb](https://github.com/nothings/stb)



## Running on Clion
Use these flags in cmake settings. i.e. set generator to vs 2022 and google test option to run MDD and MTD runtimes.
`-G "Visual Studio 17 2022" -Dgtest_force_shared_crt=ON`

## CMake recommendations
1. Avoid using file(GLOB ...) and rather list files explicitly.
2. Transitive Dependency: Engine dependency types in header. When this header is included in a project where engine is a package and this header is used. Then that engine dependency becomes dependency of the project header so needs to resolved ( Because header is copied and pasted ).
3. FETCH_CONTENT Transitive dependencies and add_subdirectory(DEPENDENCY_SOURCE_DIRECTORY) are automatically resolved when you make a package out of your project. FIND_PACKAGE or hard code paths require find dependency to resolve.
   a. Engine Dependencies that need find_dependency: BOOST, Lua,
4. make sure you have pdb (program database file) in pacakge install directory. so compilers can throw exception on code line.

## Install cmd
`cmake --install E:\OpenGL\game-engine\cmake-build-debug-visual-studio --config Debug --prefix E:/opengl/Bins/glitterEngineBincmake`

`cmake --build E:\OpenGL\game-engine\cmake-build-debug-visual-studio --target install --config Debug --prefix E:/opengl/Bins/glitterEngineBincmake`

vs code can find cpp file while debugging. ( so able to read pdb file correctly while clion cannot.)
the opened cpp file had include errors. so it probably cannot find them.