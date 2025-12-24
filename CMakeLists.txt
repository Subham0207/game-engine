cmake_minimum_required(VERSION 3.0)
project(Glitter VERSION 0.1.0 LANGUAGES C CXX)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(Glitter/Vendor/glfw)

option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(Glitter/Vendor/assimp)

add_subdirectory(Glitter/Vendor/JoltPhysics/Build)

enable_testing()

add_definitions(-DNOMINMAX)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++17")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

file(GLOB VENDORS_SOURCES Glitter/Vendor/glad/src/glad.c)
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

file(GLOB IMGUI_IMPL_SOURCES
Glitter/Vendor/imgui/*.cpp
Glitter/Vendor/imgui/backends/imgui_impl_glfw.cpp
Glitter/Vendor/imgui/backends/imgui_impl_opengl3.cpp
)

file(GLOB IMGUIZMO_IMPL_SOURCES
Glitter/Vendor/imguizmo/*.cpp
)

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

add_library(GlitterLib SHARED
        ${PROJECT_SOURCES}
        ${VENDORS_SOURCES}
        ${IMGUI_IMPL_SOURCES}
        ${IMGUIZMO_IMPL_SOURCES})

target_include_directories(GlitterLib
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Headers>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/serializations>
            $<INSTALL_INTERFACE:include>

        PRIVATE
            ${CMAKE_SOURCE_DIR}/Glitter/Vendor/imgui
            ${CMAKE_SOURCE_DIR}/Glitter/Vendor/imgui/backends
            ${CMAKE_SOURCE_DIR}/Glitter/Vendor/imguizmo
            ${CMAKE_SOURCE_DIR}/Glitter/Vendor/assimp/include/
            ${CMAKE_SOURCE_DIR}/Glitter/Vendor/glad/include/
            ${CMAKE_SOURCE_DIR}/Glitter/Vendor/glfw/include/
            ${CMAKE_SOURCE_DIR}/Glitter/Vendor/glm/
            ${CMAKE_SOURCE_DIR}/Glitter/Vendor/stb/
            ${CMAKE_SOURCE_DIR}/Glitter/Vendor/imgui/
            ${CMAKE_SOURCE_DIR}/Glitter/Vendor/imgui/backends/
            ${CMAKE_SOURCE_DIR}/Glitter/Vendor/imguizmo/
            ${CMAKE_SOURCE_DIR}/Glitter/Vendor/JoltPhysics/Jolt/
            ${Boost_INCLUDE_DIRS}
            ${LUA_INCLUDE_DIR}
            ${RECASTNAVIGATION_INCLUDE_DIRS}
)

target_link_libraries(GlitterLib
        PRIVATE
            assimp glfw
            ${GLFW_LIBRARIES} ${GLAD_LIBRARIES}
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

add_executable(ProjectManager Glitter/projectMangerMain.cpp)

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

if(MSVC)
    foreach(tgt GlitterLib Editor ProjectManager)
        set_target_properties(${tgt} PROPERTIES
                PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdb/$<CONFIG>"
                COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdb/$<CONFIG>"
        )
    endforeach()
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install the engine headers into:
#   <prefix>/include/Glitter/...
install(DIRECTORY "${CMAKE_SOURCE_DIR}/Glitter/Headers/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Glitter")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/Glitter/serializations/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Glitter/serializations")

# Install Glitter runtime + import lib:
# - RUNTIME: DLL (Windows)
# - ARCHIVE: .lib import library (Windows) / static lib
# - LIBRARY: .so/.dylib
install(TARGETS GlitterLib
        EXPORT GlitterTargets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

# Install your runtime data so new projects can run immediately
install(DIRECTORY "${CMAKE_SOURCE_DIR}/EngineAssets"
        DESTINATION ".")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/Glitter/Shaders"
        DESTINATION ".")

# If you depend on this Lua DLL at runtime, include it in the installed SDK/bin
install(FILES "${CMAKE_SOURCE_DIR}/Glitter/Vendor/lua-5.4.2/lua54.dll"
        DESTINATION "${CMAKE_INSTALL_BINDIR}")

# ---- Export targets for find_package(Glitter CONFIG) ----
# Where the package files will live:
set(GLITTER_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/Glitter")

# Export the target(s)
install(EXPORT GlitterTargets
        FILE GlitterTargets.cmake
        NAMESPACE Glitter::
        DESTINATION "${GLITTER_INSTALL_CMAKEDIR}"
)

# Generate GlitterConfig.cmake + GlitterConfigVersion.cmake
# We'll generate a tiny config template into the build tree (no extra files needed)
set(_glitter_config_in "${CMAKE_CURRENT_BINARY_DIR}/GlitterConfig.cmake.in")
file(WRITE "${_glitter_config_in}"
        "@PACKAGE_INIT@\n"
        "include(\"\${CMAKE_CURRENT_LIST_DIR}/GlitterTargets.cmake\")\n"
)

configure_package_config_file(
        "${_glitter_config_in}"
        "${CMAKE_CURRENT_BINARY_DIR}/GlitterConfig.cmake"
        INSTALL_DESTINATION "${GLITTER_INSTALL_CMAKEDIR}"
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/GlitterConfigVersion.cmake"
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY SameMajorVersion
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/GlitterConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/GlitterConfigVersion.cmake"
        DESTINATION "${GLITTER_INSTALL_CMAKEDIR}"
)

set_target_properties(GlitterLib PROPERTIES EXPORT_NAME Glitter)