cmake_minimum_required(VERSION 3.0)
project(Glitter)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(Glitter/Vendor/glfw)

option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(Glitter/Vendor/assimp)

option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_UNIT_TESTS OFF)
add_subdirectory(Glitter/Vendor/bullet)

add_subdirectory(Glitter/Vendor/JoltPhysics/Build)

enable_testing()

add_definitions(-DNOMINMAX)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

include_directories(Glitter/Headers/
                    Glitter/Vendor/assimp/include/
                    Glitter/Vendor/bullet/src/
                    Glitter/Vendor/glad/include/
                    Glitter/Vendor/glfw/include/
                    Glitter/Vendor/glm/
                    Glitter/Vendor/stb/
                    Glitter/Vendor/imgui/
                    Glitter/Vendor/imgui/backends/
                    Glitter/Vendor/imguizmo/
                    Glitter/DepsHeaderAndLibs/Headers/
                    Glitter/serializations/
                    Glitter/Vendor/JoltPhysics/Jolt/
                    )

file(GLOB VENDORS_SOURCES Glitter/Vendor/glad/src/glad.c)
file(GLOB_RECURSE PROJECT_HEADERS Glitter/Headers/*.hpp)
file(GLOB PROJECT_SOURCES Glitter/Sources/*.cpp)
file(GLOB PROJECT_SHADERS Glitter/Shaders/*.comp
                          Glitter/Shaders/*.frag
                          Glitter/Shaders/*.geom
                          Glitter/Shaders/*.vert)
file(GLOB PROJECT_CONFIGS CMakeLists.txt
                          Readme.md
                         .gitattributes
                         .gitignore
                         .gitmodules)

file(GLOB IMGUI_IMPL_HEADERS 
Glitter/Vendor/imgui/*.h
Glitter/Vendor/imgui/backends/imgui_impl_glfw.h
Glitter/Vendor/imgui/backends/imgui_impl_opengl3.h
)

file(GLOB IMGUI_IMPL_SOURCES
Glitter/Vendor/imgui/*.cpp
Glitter/Vendor/imgui/backends/imgui_impl_glfw.cpp
Glitter/Vendor/imgui/backends/imgui_impl_opengl3.cpp
)

file(GLOB IMGUIZMO_IMPL_HEADERS 
Glitter/Vendor/imguizmo/*.h
)
file(GLOB IMGUIZMO_IMPL_SOURCES
Glitter/Vendor/imguizmo/*.cpp
)

                    

source_group("Headers" FILES ${PROJECT_HEADERS})
source_group("Shaders" FILES ${PROJECT_SHADERS})
source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("Vendors" FILES ${VENDORS_SOURCES})

set(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Glitter/Vendor/boost_1_89_0")
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_DEBUG ON)

find_package(Boost 1.89.0 REQUIRED COMPONENTS serialization)

add_definitions(-DGLFW_INCLUDE_NONE
                -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")


set(LUA_INCLUDE_DIR
        "${CMAKE_SOURCE_DIR}/Glitter/Vendor/lua-5.4.2/include/"
        CACHE PATH "The path to the Lua headers" FORCE)
set(LUA_LIBRARY
        "${CMAKE_SOURCE_DIR}/Glitter/Vendor/lua-5.4.2/lua54.lib"
        CACHE FILEPATH "The full path to the Lua library file" FORCE)
set(LUA_DLL
        "${CMAKE_SOURCE_DIR}/Glitter/Vendor/lua-5.4.2/lua54.dll"
        CACHE FILEPATH "The full path to the Lua library file" FORCE)

if (NOT TARGET Lua::Lua)
    add_library(Lua::Lua SHARED IMPORTED)
    set_target_properties(Lua::Lua PROPERTIES
            IMPORTED_LOCATION                   "${LUA_DLL}"
            IMPORTED_IMPLIB               "${LUA_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
    )
endif()

include(FetchContent)
FetchContent_Declare(
        sol2
        GIT_REPOSITORY https://github.com/ThePhD/sol2.git
        GIT_TAG        v3.3.0
)
FetchContent_MakeAvailable(sol2)

set(RECASTNAVIGATION_DEMO OFF CACHE BOOL "" FORCE)
set(RECASTNAVIGATION_TESTS OFF CACHE BOOL "" FORCE)
set(RECASTNAVIGATION_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RECASTNAVIGATION_DEBUGUTILS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        recast
        GIT_REPOSITORY https://github.com/recastnavigation/recastnavigation.git
        GIT_TAG v1.6.0
)

FetchContent_MakeAvailable(recast)

FetchContent_GetProperties(recast)
if (NOT recast_POPULATED)
    FetchContent_Populate(recast)
endif()

set(RECASTNAVIGATION_INCLUDE_DIRS
        ${recast_SOURCE_DIR}/Recast/Include
        ${recast_SOURCE_DIR}/Detour/Include
        ${recast_SOURCE_DIR}/DetourCrowd/Include
        ${recast_SOURCE_DIR}/DetourTileCache/Include
)

add_library(GlitterLib ${PROJECT_SOURCES}
        ${PROJECT_SHADERS} ${PROJECT_CONFIGS}
          ${VENDORS_SOURCES} ${IMGUI_IMPL_SOURCES} ${IMGUIZMO_IMPL_SOURCES})

target_include_directories(GlitterLib
        PUBLIC
        ${PROJECT_HEADERS}
        ${IMGUI_IMPL_HEADERS}
        ${IMGUIZMO_IMPL_HEADERS}
        ${Boost_INCLUDE_DIRS}
        ${LUA_INCLUDE_DIR}
        ${RECASTNAVIGATION_INCLUDE_DIRS}
)

target_link_libraries(GlitterLib assimp glfw
        ${GLFW_LIBRARIES} ${GLAD_LIBRARIES}
        BulletDynamics BulletCollision LinearMath
        Jolt::Jolt
        Boost::serialization
        Lua::Lua
        sol2::sol2

        Recast
        Detour
        DetourCrowd
        DetourTileCache
)

add_executable(Editor Glitter/editorMain.cpp)

target_link_libraries(Editor PRIVATE GlitterLib)

add_executable(ProjectManager Glitter/main.cpp)

target_link_libraries(ProjectManager PRIVATE GlitterLib)

add_subdirectory(Tests)

add_custom_command(TARGET ProjectManager POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/EngineAssets"
          "$<TARGET_FILE_DIR:ProjectManager>/EngineAssets"
)

add_custom_command(TARGET ProjectManager POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/Glitter/Shaders"
          "$<TARGET_FILE_DIR:ProjectManager>/Shaders"
)

# copy lua dll into exe directory
add_custom_command(TARGET ProjectManager POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
          "${CMAKE_SOURCE_DIR}/Glitter/Vendor/lua-5.4.2/lua54.dll"
          "$<TARGET_FILE_DIR:ProjectManager>/"
)
