// Local Headers
#include "glitter.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>

#include <model.hpp>
#include <shader.hpp>
#include <Input.hpp>
#include <Camera.hpp>

#include <Helper.hpp>

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

struct MaterialProp {
    GLuint diffuseTexture;
    GLuint specularTexture;
    GLuint shader;
    GLuint vao;
    GLuint vbo;
} materialProp1, lightMaterialProp1;

struct ClientHandler {
    InputHandler* inputHandler;
    Camera* camera;

} clientHandler;

// positions all containers
glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

glm::vec3 pointLightPositions[] = {
glm::vec3(0.7f,  0.2f,  2.0f),
glm::vec3(2.3f, -3.3f, -4.0f),
glm::vec3(-4.0f,  2.0f, -12.0f),
glm::vec3(0.0f,  0.0f, -3.0f)
};

static unsigned int LoadTexture(const char* filename, unsigned int textureUnitOffset, bool isRGBA = false)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0 + textureUnitOffset);// total of 16 textures can be activated at once; 2nd texture unit = GL_TEXTURE0 + 2
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, isRGBA ? GL_RGBA : GL_RGB, width, height, 0, isRGBA ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return texture;
}

static void MyInit(GLFWwindow* window)
{
    // Camera Init View and projection
    clientHandler.camera = new Camera();

    //Input handler
    clientHandler.inputHandler = new InputHandler(clientHandler.camera, window, 800, 600);
    InputHandler::currentInputHandler = clientHandler.inputHandler;


    // VAO

    glGenVertexArrays(1, &materialProp1.vao);
    glBindVertexArray(materialProp1.vao);

    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenBuffers(1, &materialProp1.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, materialProp1.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);// vertexAttrib, no of comp, type, normalize?, stride, offset
    //glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Texture1
    materialProp1.diffuseTexture = LoadTexture("E:/OpenGL/Glitter/Res/Textures/container2.png", 0, true);
    //Texture1
    materialProp1.specularTexture = LoadTexture("E:/OpenGL/Glitter/Res/Textures/container2_specular.png", 1, true);
    //Texture2
    //unsigned int emissionTexture = LoadTexture("Res/Textures/matrix.jpg", 2, false);

    Helper::ShaderProgramSource source = Helper::ParseShader("E:/OpenGL/Glitter/Res/Shader/Basic.shader"); // file path from root of the -- working directory in debug mode is ProjectRootDirectory
    materialProp1.shader = Helper::CreateShader(source.VertexSource, source.FragmentSource);
    glUseProgram(materialProp1.shader);


    //Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    Helper::unbindAllOpenGLObjects();

    // VAO end .....

    //uniform
    //glUseProgram(shader);
    //unsigned int cubeModelLoc = glGetUniformLocation(shader, "model");
    //glUniform3f(glGetUniformLocation(shader, "lightColor"), 1.0f,1.0f,1.0f);
    //glUniform3f(glGetUniformLocation(shader, "objectColor"), 1.0f, 0.5f, 0.31f);
    //glUseProgram(0);

    // Light source VAO

    glGenVertexArrays(1, &lightMaterialProp1.vao);
    glBindVertexArray(lightMaterialProp1.vao);

    glGenBuffers(1, &lightMaterialProp1.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, lightMaterialProp1.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    Helper::ShaderProgramSource LightShaderSource = Helper::ParseShader("Res/Shader/LightSource.shader"); // file path from root of the -- working directory in debug mode is ProjectRootDirectory
    lightMaterialProp1.shader = Helper::CreateShader(LightShaderSource.VertexSource, LightShaderSource.FragmentSource);
    glUseProgram(lightMaterialProp1.shader);
    //Uniforms
    //unsigned int LightSourceModelLoc = glGetUniformLocation(Lightshader, "model");

    // Light Source VAO end ....

    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

    //Enable ZDepth-Test
    glEnable(GL_DEPTH_TEST);

    // Unbinding every OpenGL object
    Helper::unbindAllOpenGLObjects();

    //wireframe mode
    Helper::switchOnWireFrame(false);
    // max number of vertex attributes available
    Helper::maxVertexAttributesAvailableOnMyGPU();

}

static void MyGameLoop()
{
    //delta time -- making things time dependent
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Applies input to the camera
    clientHandler.inputHandler->handleInput(deltaTime);

    // Cube
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, materialProp1.diffuseTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, materialProp1.specularTexture);
    glActiveTexture(GL_TEXTURE2);
    //glBindTexture(GL_TEXTURE_2D, emissionTexture);
    glUseProgram(materialProp1.shader);
    clientHandler.camera->updateMVP(materialProp1.shader);
    glBindVertexArray(materialProp1.vao);
    unsigned int cubeModelLoc = glGetUniformLocation(materialProp1.shader, "model");
    glUniform3f(glGetUniformLocation(materialProp1.shader, "viewPos"), clientHandler.camera->getPosition().r, clientHandler.camera->getPosition().g, clientHandler.camera->getPosition().b);

    glUniform1i(glGetUniformLocation(materialProp1.shader, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(materialProp1.shader, "material.specular"), 1);
    //glUniform1i(glGetUniformLocation(shader, "material.emission"), 2);
    glUniform1f(glGetUniformLocation(materialProp1.shader, "material.shininess"), 32.0f);

    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
    glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);

    //DirectionLight properties
    glUniform3f(glGetUniformLocation(materialProp1.shader, "dirLight.direction"), 0.5f, 0.5f, 0.5f);
    glUniform3f(glGetUniformLocation(materialProp1.shader, "dirLight.ambient"), ambientColor.x, ambientColor.y, ambientColor.z);
    glUniform3f(glGetUniformLocation(materialProp1.shader, "dirLight.diffuse"), diffuseColor.x, diffuseColor.y, diffuseColor.z);
    glUniform3f(glGetUniformLocation(materialProp1.shader, "dirLight.specular"), 1.0f, 1.0f, 1.0f);

    // 4 point lights
    for (unsigned int i = 0; i < 4; i++)
    {
        std::stringstream pointLightPosition_ss;
        pointLightPosition_ss << "pointLights[" << i << "].position";
        glUniform3f(glGetUniformLocation(materialProp1.shader, pointLightPosition_ss.str().c_str()), pointLightPositions[i].r, pointLightPositions[i].g, pointLightPositions[i].b);
        std::stringstream pointLightAmbient_ss;
        pointLightAmbient_ss << "pointLights[" << i << "].ambient";
        glUniform3f(glGetUniformLocation(materialProp1.shader, pointLightAmbient_ss.str().c_str()), ambientColor.x, ambientColor.y, ambientColor.z);
        std::stringstream pointLightdiffuse_ss;
        pointLightdiffuse_ss << "pointLights[" << i << "].diffuse";
        glUniform3f(glGetUniformLocation(materialProp1.shader, pointLightdiffuse_ss.str().c_str()), diffuseColor.x, diffuseColor.y, diffuseColor.z);
        std::stringstream pointLightSpecular_ss;
        pointLightdiffuse_ss << "pointLights[" << i << "].specular";
        glUniform3f(glGetUniformLocation(materialProp1.shader, pointLightdiffuse_ss.str().c_str()), 1.0f, 1.0f, 1.0f);
        std::stringstream pointLightConstantTerm_ss;
        pointLightConstantTerm_ss << "pointLights[" << i << "].constantTerm";
        glUniform1f(glGetUniformLocation(materialProp1.shader, pointLightConstantTerm_ss.str().c_str()), 1.0f);
        std::stringstream pointLightLinearTerm_ss;
        pointLightLinearTerm_ss << "pointLights[" << i << "].linearTerm";
        glUniform1f(glGetUniformLocation(materialProp1.shader, pointLightLinearTerm_ss.str().c_str()), 0.09f);
        std::stringstream pointLightQuadraticTerm_ss;
        pointLightQuadraticTerm_ss << "pointLights[" << i << "].quadraticTerm";
        glUniform1f(glGetUniformLocation(materialProp1.shader, pointLightQuadraticTerm_ss.str().c_str()), 0.032f);
    }

    //SpotLight as well
    glUniform3f(glGetUniformLocation(materialProp1.shader, "spotLight.position"), clientHandler.camera->getPosition().r, clientHandler.camera->getPosition().g, clientHandler.camera->getPosition().b);
    glUniform3f(glGetUniformLocation(materialProp1.shader, "spotLight.direction"), clientHandler.camera->getFront().r, clientHandler.camera->getFront().g, clientHandler.camera->getFront().b);
    glUniform1f(glGetUniformLocation(materialProp1.shader, "spotLight.cutoff"), glm::cos(glm::radians(12.5f)));
    glUniform1f(glGetUniformLocation(materialProp1.shader, "spotLight.outerCutOff"), glm::cos(glm::radians(17.5f)));
    glUniform3f(glGetUniformLocation(materialProp1.shader, "spotLight.ambient"), ambientColor.x, ambientColor.y, ambientColor.z);
    glUniform3f(glGetUniformLocation(materialProp1.shader, "spotLight.diffuse"), diffuseColor.x, diffuseColor.y, diffuseColor.z);
    glUniform3f(glGetUniformLocation(materialProp1.shader, "spotLight.specular"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(materialProp1.shader, "spotLight.constantTerm"), 1.0f);
    glUniform1f(glGetUniformLocation(materialProp1.shader, "spotLight.linearTerm"), 0.09f);
    glUniform1f(glGetUniformLocation(materialProp1.shader, "spotLight.quadraticTerm"), 0.032f);

    for (unsigned int i = 0; i < 10; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        model = glm::rotate(model, 20.0f * i, glm::vec3(1.0f, 0.3f, 0.5f));
        glUniformMatrix4fv(cubeModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    // Cube end ....

    // Cube light source
    glUseProgram(lightMaterialProp1.shader);
    clientHandler.camera->updateMVP(lightMaterialProp1.shader);
    glBindVertexArray(lightMaterialProp1.vao);
    unsigned int LightSourceModelLoc = glGetUniformLocation(lightMaterialProp1.shader, "model");
    for (unsigned int i = 0; i < 4; i++)
    {
        glm::mat4 Lightmodel = glm::mat4(1.0f);
        Lightmodel = glm::translate(Lightmodel, pointLightPositions[i]);
        Lightmodel = glm::scale(Lightmodel, glm::vec3(0.3f, 0.3f, 0.3f));
        glUniformMatrix4fv(LightSourceModelLoc, 1, GL_FALSE, glm::value_ptr(Lightmodel));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    // Cube light source end ....
}


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

    glfwSetFramebufferSizeCallback(mWindow, Helper::frame_buffer_size_callback);

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    //disable mouse pointer
    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //You init code goes inside this function...
    MyInit(mWindow);

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Your game loop code goes inside this function...
        MyGameLoop();

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   glfwTerminate();
    return EXIT_SUCCESS;
}
