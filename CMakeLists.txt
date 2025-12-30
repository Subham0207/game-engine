cmake_minimum_required(VERSION 3.0)
project(Glitter VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

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

if(NOT APPLE)
    set(CMAKE_INSTALL_RPATH $ORIGIN)
endif ()

add_library(GlitterLib STATIC
        ${PROJECT_SOURCES}
        ${VENDORS_SOURCES}
        ${IMGUI_IMPL_SOURCES}
        ${IMGUIZMO_IMPL_SOURCES})

target_include_directories(GlitterLib
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Headers>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/serializations>

            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Vendor/imgui>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Vendor/imgui/backends>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Vendor/imguizmo>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Vendor/assimp/include/>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Vendor/glad/include/>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Vendor/glfw/include/>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Vendor/glm/>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Vendor/stb/>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Vendor/imgui/>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Vendor/imgui/backends/>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Vendor/imguizmo/>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Glitter/Vendor/JoltPhysics/Jolt/>
            $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>
            $<BUILD_INTERFACE:${LUA_INCLUDE_DIR}>
            $<BUILD_INTERFACE:${RECASTNAVIGATION_INCLUDE_DIRS}>
            $<INSTALL_INTERFACE:include/Glitter>
)

target_link_libraries(GlitterLib
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

add_executable(Editor ${CMAKE_SOURCE_DIR}/Glitter/editorMain.cpp)

target_link_libraries(Editor PRIVATE GlitterLib)

add_executable(ProjectManager ${CMAKE_SOURCE_DIR}/Glitter/projectMangerMain.cpp)

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

add_custom_command(TARGET ProjectManager POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/Glitter/Template"
          "$<TARGET_FILE_DIR:ProjectManager>/Template"
)

# copy lua dll into exe directory
add_custom_command(TARGET ProjectManager POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
          "${CMAKE_SOURCE_DIR}/Glitter/Vendor/lua-5.4.2/lua54.dll"
          "$<TARGET_FILE_DIR:ProjectManager>/"
)

# --- 7. Installation & Packaging ---
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(GLITTER_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/Glitter")

# UNIFIED INSTALL TARGETS (Only call this ONCE per target)
install(TARGETS GlitterLib assimp zlibstatic glfw Jolt Recast Detour DetourCrowd DetourTileCache sol2
        EXPORT GlitterTargets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

set_target_properties(GlitterLib PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

# Install Headers
install(DIRECTORY "${CMAKE_SOURCE_DIR}/Glitter/Headers/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Glitter")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/Glitter/Vendor/lua-5.4.2/include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Glitter/Vendor/lua")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/Glitter/Vendor/JoltPhysics/Jolt" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(DIRECTORY "${recast_SOURCE_DIR}/Recast/Include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Recast")
install(DIRECTORY "${recast_SOURCE_DIR}/Detour/Include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Detour")

# Install Bundled Dependencies
install(FILES "${LUA_LIBRARY}" DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES "${LUA_DLL}" DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(DIRECTORY "${BOOST_ROOT}/boost" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(DIRECTORY "${BOOST_ROOT}/stage/lib/" DESTINATION "${CMAKE_INSTALL_LIBDIR}" FILES_MATCHING PATTERN "*.lib")

# --- 8. Generate Package Config ---
set(_glitter_config_in "${CMAKE_CURRENT_BINARY_DIR}/GlitterConfig.cmake.in")
file(WRITE "${_glitter_config_in}" [=[
@PACKAGE_INIT@
include(CMakeFindDependencyMacro)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_dependency(Threads)

set(BOOST_ROOT "${PACKAGE_PREFIX_DIR}")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_NO_SYSTEM_PATHS ON)
find_dependency(Boost 1.89.0 REQUIRED COMPONENTS serialization)

if (NOT TARGET Lua::Lua)
    add_library(Lua::Lua SHARED IMPORTED)
    set_target_properties(Lua::Lua PROPERTIES
        IMPORTED_LOCATION "${PACKAGE_PREFIX_DIR}/bin/lua54.dll"
        IMPORTED_IMPLIB "${PACKAGE_PREFIX_DIR}/lib/lua54.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/include/Glitter/Vendor/lua")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/GlitterTargets.cmake")
]=])

install(DIRECTORY "${CMAKE_SOURCE_DIR}/EngineAssets/"
        DESTINATION EngineAssets)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/Glitter/Shaders/"
        DESTINATION Shaders)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/Glitter/Template/"
        DESTINATION Template)

configure_package_config_file("${_glitter_config_in}" "${CMAKE_CURRENT_BINARY_DIR}/GlitterConfig.cmake" INSTALL_DESTINATION "${GLITTER_INSTALL_CMAKEDIR}")
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/GlitterConfigVersion.cmake" VERSION "${PROJECT_VERSION}" COMPATIBILITY SameMajorVersion)

install(EXPORT GlitterTargets FILE GlitterTargets.cmake NAMESPACE Glitter:: DESTINATION "${GLITTER_INSTALL_CMAKEDIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GlitterConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/GlitterConfigVersion.cmake" DESTINATION "${GLITTER_INSTALL_CMAKEDIR}")

set_target_properties(GlitterLib PROPERTIES EXPORT_NAME Glitter)