// Local Headers
#include "glitter.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>

#include <mesh.hpp>
#include <shader.hpp>
#include <Camera.hpp>
#include <Input.hpp>

#include <sstream>

struct ClientHandler {
    InputHandler* inputHandler;
    Camera* camera;

} clientHandler;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

glm::vec3 pointLightPositions[] = {
glm::vec3(0.7f,  0.2f,  2.0f),
glm::vec3(2.3f, -3.3f, -4.0f),
glm::vec3(-4.0f,  2.0f, -12.0f),
glm::vec3(0.0f,  0.0f, -3.0f)
};


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
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    clientHandler.camera = new Camera();
    clientHandler.inputHandler = new InputHandler(clientHandler.camera, mWindow, 800, 600);
    InputHandler::currentInputHandler = clientHandler.inputHandler;

    auto model1 = new Mirage::Mesh("4BarrelGunTianglulated.fbx");
    auto shaderProgram = new Mirage::Shader();
    shaderProgram->attach("Basic.vert").attach("Basic.frag");

    glEnable(GL_DEPTH_TEST);
    
    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        clientHandler.inputHandler->handleInput(deltaTime);

        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //Render
        model1->draw(shaderProgram->get());
        clientHandler.camera->updateMVP(shaderProgram->get());
        unsigned int cubeModelLoc = glGetUniformLocation(shaderProgram->get(), "model");
        glUniform3f(glGetUniformLocation(shaderProgram->get(), "viewPos"), clientHandler.camera->getPosition().r, clientHandler.camera->getPosition().g, clientHandler.camera->getPosition().b);
        glUniform1i(glGetUniformLocation(shaderProgram->get(), "material.diffuse"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram->get(), "material.specular"), 1);
        glUniform1f(glGetUniformLocation(shaderProgram->get(), "material.shininess"), 32.0f);
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);

        //DirectionLight properties
        glUniform3f(glGetUniformLocation(shaderProgram->get(), "dirLight.direction"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(shaderProgram->get(), "dirLight.ambient"), ambientColor.x, ambientColor.y, ambientColor.z);
        glUniform3f(glGetUniformLocation(shaderProgram->get(), "dirLight.diffuse"), diffuseColor.x, diffuseColor.y, diffuseColor.z);
        glUniform3f(glGetUniformLocation(shaderProgram->get(), "dirLight.specular"), 1.0f, 1.0f, 1.0f);

        // 4 point lights
        for (unsigned int i = 0; i < 4; i++)
        {
            std::stringstream pointLightPosition_ss;
            pointLightPosition_ss << "pointLights[" << i << "].position";
            glUniform3f(glGetUniformLocation(shaderProgram->get(), pointLightPosition_ss.str().c_str()), pointLightPositions[i].r, pointLightPositions[i].g, pointLightPositions[i].b);
            std::stringstream pointLightAmbient_ss;
            pointLightAmbient_ss << "pointLights[" << i << "].ambient";
            glUniform3f(glGetUniformLocation(shaderProgram->get(), pointLightAmbient_ss.str().c_str()), ambientColor.x, ambientColor.y, ambientColor.z);
            std::stringstream pointLightdiffuse_ss;
            pointLightdiffuse_ss << "pointLights[" << i << "].diffuse";
            glUniform3f(glGetUniformLocation(shaderProgram->get(), pointLightdiffuse_ss.str().c_str()), diffuseColor.x, diffuseColor.y, diffuseColor.z);
            std::stringstream pointLightSpecular_ss;
            pointLightdiffuse_ss << "pointLights[" << i << "].specular";
            glUniform3f(glGetUniformLocation(shaderProgram->get(), pointLightdiffuse_ss.str().c_str()), 1.0f, 1.0f, 1.0f);
            std::stringstream pointLightConstantTerm_ss;
            pointLightConstantTerm_ss << "pointLights[" << i << "].constantTerm";
            glUniform1f(glGetUniformLocation(shaderProgram->get(), pointLightConstantTerm_ss.str().c_str()), 1.0f);
            std::stringstream pointLightLinearTerm_ss;
            pointLightLinearTerm_ss << "pointLights[" << i << "].linearTerm";
            glUniform1f(glGetUniformLocation(shaderProgram->get(), pointLightLinearTerm_ss.str().c_str()), 0.09f);
            std::stringstream pointLightQuadraticTerm_ss;
            pointLightQuadraticTerm_ss << "pointLights[" << i << "].quadraticTerm";
            glUniform1f(glGetUniformLocation(shaderProgram->get(), pointLightQuadraticTerm_ss.str().c_str()), 0.032f);
        }

        //SpotLight as well
        glUniform3f(glGetUniformLocation(shaderProgram->get(), "spotLight.position"), clientHandler.camera->getPosition().r, clientHandler.camera->getPosition().g, clientHandler.camera->getPosition().b);
        glUniform3f(glGetUniformLocation(shaderProgram->get(), "spotLight.direction"), clientHandler.camera->getFront().r, clientHandler.camera->getFront().g, clientHandler.camera->getFront().b);
        glUniform1f(glGetUniformLocation(shaderProgram->get(), "spotLight.cutoff"), glm::cos(glm::radians(12.5f)));
        glUniform1f(glGetUniformLocation(shaderProgram->get(), "spotLight.outerCutOff"), glm::cos(glm::radians(17.5f)));
        glUniform3f(glGetUniformLocation(shaderProgram->get(), "spotLight.ambient"), ambientColor.x, ambientColor.y, ambientColor.z);
        glUniform3f(glGetUniformLocation(shaderProgram->get(), "spotLight.diffuse"), diffuseColor.x, diffuseColor.y, diffuseColor.z);
        glUniform3f(glGetUniformLocation(shaderProgram->get(), "spotLight.specular"), 1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(shaderProgram->get(), "spotLight.constantTerm"), 1.0f);
        glUniform1f(glGetUniformLocation(shaderProgram->get(), "spotLight.linearTerm"), 0.09f);
        glUniform1f(glGetUniformLocation(shaderProgram->get(), "spotLight.quadraticTerm"), 0.032f);


        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   glfwTerminate();
    return EXIT_SUCCESS;
}
