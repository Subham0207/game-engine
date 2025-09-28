## Getting Started
Start by cloning this repository, making sure to pass the `--recursive` flag to grab all the dependencies. 
If you forgot, then you can `git submodule update --init` instead.

```bash
git clone --recursive https://github.com/Polytonic/Glitter
cd Glitter
cd Build
```

# Integrating Boost library
1. download source code from https://www.boost.org/releases/latest/.
2. extract file and put in under ./Glitter/Vendor/
3. cd into boost directory. For more detail follow https://www.boost.org/doc/user-guide/getting-started.html or run following cmds
    run booststrap.bat
    then b2.exe to compile lib. And then
    b2 install --prefix=../Boost_install
4. Note the version of boost is what is reference in cmakelist of your project.


Now generate a project file or makefile for your platform. If you want to use a particular IDE, make sure it is installed; don't forget to set the Start-Up Project in Visual Studio or the Target in Xcode.

```bash
# UNIX Makefile
cmake ..

# Mac OSX
cmake -G "Xcode" ..

# Microsoft Windows
cmake -G "Visual Studio 14" ..
cmake -G "Visual Studio 14 Win64" ..
...
```

If you compile and run, you should now be at the same point as the [Hello Window](http://www.learnopengl.com/#!Getting-started/Hello-Window) or [Context Creation](https://open.gl/context) sections of the tutorials. Open [main.cpp](https://github.com/Polytonic/Glitter/blob/master/Glitter/Sources/main.cpp) on your computer and start writing code!

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
