#pragma once
// Local Headers
#include "glitter.hpp"

// System Headers
#include <GLFW/glfw3.h>
#include <glad/glad.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>

#include "basicMesh.hpp"
#include "camera.hpp"

#include <shader.hpp>

#include <imgui.h>
#include <useimgui.hpp>

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440

int main(int argc, char * argv[]) {

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    // commenting this becauase GL_VERSION macro is defined at two places namely glad.h and imgui_loader.h
    //fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    //Init

    Shader* shader = new Shader();
    shader->CompileShaders();
    
    BasicMesh* mesh = new BasicMesh();
    mesh->LoadMesh("E:/OpenGL/4barrel.fbx");

    PersProjInfo perspProjInfo{ 45.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 1000000000.0f };

    Vector3f CameraPos(0.0f, 0.0f, -500.0f);
    Vector3f CameraTarget(0.0f, 0.0f, 1.0f);
    Vector3f CameraUp(0.0f, 1.0f, 0.0f);

    Camera* pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, CameraPos, CameraTarget, CameraUp);

    auto myGUI = new UseImGui();
    myGUI->Init(mWindow, "#version 400");

    // Init end...

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float YRotationAngle = 1.0f;

        //Render
        WorldTrans& worldTransform = mesh->GetWorldTransform();
        worldTransform.SetScale(0.01f);
        worldTransform.SetPosition(0.0f, 0.0f, 2.0f);
        worldTransform.Rotate(0.0f, YRotationAngle, 0.0f);
        Matrix4f World = worldTransform.GetMatrix();
        Matrix4f View = pGameCamera->GetMatrix();
        Matrix4f Projection;
        Projection.InitPersProjTransform(perspProjInfo);

        Matrix4f WVP = Projection * View * World;
        glUniformMatrix4fv(shader->Model, 1, GL_FALSE, &World.m[0][0]);
        glUniformMatrix4fv(shader->View, 1, GL_FALSE, &View.m[0][0]);
        glUniformMatrix4fv(shader->Projection, 1, GL_FALSE, &Projection.m[0][0]);
        glUniform1i(shader->SamplerLocation, 0);

        pGameCamera->OnRender();


        mesh->Render();

        myGUI->NewFrame();
        myGUI->Update();
        myGUI->Render();


        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   glfwTerminate();
    myGUI->Shutdown();
    return EXIT_SUCCESS;
}
