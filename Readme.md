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
    run booststrap.bat
    then b2.exe to compile lib. And then
    b2 install --prefix=../Boost_install
4. Note the version of boost is what is referenced in cmakelist of your project. So if you downloded a different version update that.

Now, run build from vs code or execute `.\run_debug.bat` to generate build files
Then make sure in build/cmakecache.txt to make -- `gtest_force_shared_crt:BOOL=ON`. Read More detail on this in Documention file of this project.

# Integrating Lua and sol2 libraries:
Lua is a scripting library. Sol2 simplifies the use of Lua.
1. Download binaries of Lua from https://luabinaries.sourceforge.net/
2. Place it under ./Glitter/Vendor/ and things should work
3. sol2 is being fetched by project cmakefile using fetch content.

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
