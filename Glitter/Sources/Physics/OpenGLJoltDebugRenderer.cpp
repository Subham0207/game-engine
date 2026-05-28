#include <Physics/OpenGLJoltDebugRenderer.hpp>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

namespace
{
    constexpr const char* kDebugLineVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPosition;
        layout (location = 1) in vec4 aColor;

        uniform mat4 uViewProjection;

        out vec4 vColor;

        void main()
        {
            gl_Position = uViewProjection * vec4(aPosition, 1.0);
            vColor = aColor;
        }
    )";

    constexpr const char* kDebugLineFragmentShader = R"(
        #version 330 core
        in vec4 vColor;
        out vec4 FragColor;

        void main()
        {
            FragColor = vColor;
        }
    )";
}

Physics::OpenGLJoltDebugRenderer::OpenGLJoltDebugRenderer()
{
    initGpuResources();
}

Physics::OpenGLJoltDebugRenderer::~OpenGLJoltDebugRenderer()
{
    if (mVbo != 0)
    {
        glDeleteBuffers(1, &mVbo);
        mVbo = 0;
    }

    if (mVao != 0)
    {
        glDeleteVertexArrays(1, &mVao);
        mVao = 0;
    }

    if (mProgram != 0)
    {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
}

void Physics::OpenGLJoltDebugRenderer::beginFrame()
{
    mLineVertices.clear();
}

void Physics::OpenGLJoltDebugRenderer::render(const glm::mat4& viewProjection)
{
    if (mProgram == 0 || mVao == 0 || mVbo == 0 || mLineVertices.empty())
    {
        return;
    }

    glEnable(GL_DEPTH_TEST);
    GLboolean previousDepthWriteMask = GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthWriteMask);
    glDepthMask(GL_FALSE);

    glUseProgram(mProgram);
    glUniformMatrix4fv(mViewProjectionLocation, 1, GL_FALSE, glm::value_ptr(viewProjection));

    glBindVertexArray(mVao);
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(mLineVertices.size() * sizeof(LineVertex)),
        mLineVertices.data(),
        GL_DYNAMIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), reinterpret_cast<void*>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(LineVertex), reinterpret_cast<void*>(3 * sizeof(float)));

    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(mLineVertices.size()));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glDepthMask(previousDepthWriteMask);
}

void Physics::OpenGLJoltDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
    const float colorScale = 1.0f / 255.0f;
    JPH::Color lineColor = inColor;
    if (lineColor == JPH::Color::sGrey || lineColor == JPH::Color::sLightGrey)
    {
        lineColor = JPH::Color::sYellow;
    }

    LineVertex a{};
    a.px = static_cast<float>(inFrom.GetX());
    a.py = static_cast<float>(inFrom.GetY());
    a.pz = static_cast<float>(inFrom.GetZ());
    a.cr = static_cast<float>(lineColor.r) * colorScale;
    a.cg = static_cast<float>(lineColor.g) * colorScale;
    a.cb = static_cast<float>(lineColor.b) * colorScale;
    a.ca = static_cast<float>(lineColor.a) * colorScale;

    LineVertex b{};
    b.px = static_cast<float>(inTo.GetX());
    b.py = static_cast<float>(inTo.GetY());
    b.pz = static_cast<float>(inTo.GetZ());
    b.cr = a.cr;
    b.cg = a.cg;
    b.cb = a.cb;
    b.ca = a.ca;

    mLineVertices.push_back(a);
    mLineVertices.push_back(b);
}

void Physics::OpenGLJoltDebugRenderer::DrawText3D(JPH::RVec3Arg inPosition, const JPH::string_view& inString, JPH::ColorArg inColor, float inHeight)
{
    // No-op: text overlay is currently handled by ImGui/UI systems in the engine.
}

void Physics::OpenGLJoltDebugRenderer::initGpuResources()
{
    const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, kDebugLineVertexShader);
    const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, kDebugLineFragmentShader);

    mProgram = glCreateProgram();
    glAttachShader(mProgram, vertexShader);
    glAttachShader(mProgram, fragmentShader);
    glLinkProgram(mProgram);

    int linkStatus = 0;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE)
    {
        char infoLog[1024]{};
        glGetProgramInfoLog(mProgram, static_cast<GLsizei>(sizeof(infoLog)), nullptr, infoLog);
        std::cerr << "[PhysicsDebug] Failed to link Jolt debug renderer shader program: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGenVertexArrays(1, &mVao);
    glGenBuffers(1, &mVbo);

    mViewProjectionLocation = glGetUniformLocation(mProgram, "uViewProjection");
}

unsigned int Physics::OpenGLJoltDebugRenderer::compileShader(const unsigned int shaderType, const char* source)
{
    const unsigned int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int compileStatus = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        char infoLog[1024]{};
        glGetShaderInfoLog(shader, static_cast<GLsizei>(sizeof(infoLog)), nullptr, infoLog);
        std::cerr << "[PhysicsDebug] Failed to compile Jolt debug shader: " << infoLog << std::endl;
    }

    return shader;
}

