#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRendererSimple.h>
#include <glm/glm.hpp>

#include <vector>

namespace Physics
{
    class OpenGLJoltDebugRenderer final : public JPH::DebugRendererSimple
    {
    public:
        OpenGLJoltDebugRenderer();
        ~OpenGLJoltDebugRenderer() override;

        void beginFrame();
        void render(const glm::mat4& viewProjection);

        void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
        void DrawText3D(JPH::RVec3Arg inPosition, const JPH::string_view& inString, JPH::ColorArg inColor, float inHeight) override;

    private:
        struct LineVertex
        {
            float px;
            float py;
            float pz;
            float cr;
            float cg;
            float cb;
            float ca;
        };

        unsigned int mVao = 0;
        unsigned int mVbo = 0;
        unsigned int mProgram = 0;
        int mViewProjectionLocation = -1;

        std::vector<LineVertex> mLineVertices;

        void initGpuResources();
        static unsigned int compileShader(unsigned int shaderType, const char* source);
    };
}


