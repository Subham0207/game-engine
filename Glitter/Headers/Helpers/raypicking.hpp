#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/intersect.hpp"
#include <vector>
#include "imgui.h"

#ifdef max
#undef max
#endif

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <Renderable/renderable.hpp>

void renderRay(const glm::vec3& rayOrigin, const glm::vec3& rayDir, unsigned int shaderId);
void renderRayWithIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayEnd, unsigned int shaderId);
bool rayPlaneHit(
    const glm::vec3& O, const glm::vec3& D,
    const glm::vec3& P, const glm::vec3& N,
    float& tOut, glm::vec3& X
);

int selectModel(const glm::vec3& rayOrigin, const glm::vec3& rayDir, glm::vec3& rayEnd, const std::vector<std::shared_ptr<Renderable>>& renderables);

glm::vec3 getNormalizedCoordinateForDepth(double winX, double winY, double screenWidth, double screenHeight, double depth);

void setRay(double winX, double winY, glm::vec3& rayOrigin, glm::vec3& rayDir, glm::mat4 &glmModelView, glm::mat4 &glmProjection, glm::vec3 cameraDirection);

int handlePicking(
    double mouseX,
    double mouseY,
    const std::vector<std::shared_ptr<Renderable>>& renderables,
    glm::mat4 &view,
    glm::mat4 &projection,
    unsigned int rayShader,
    glm::vec3 &rayOrigin,
    glm::vec3 &rayDir,
    glm::vec3 cameraDirection);