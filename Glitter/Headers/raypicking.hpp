#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/intersect.hpp>
#include <vector>
#include "Model.hpp"
#include "imgui.h"

#ifdef max
#undef max
#endif
#include <limits>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void renderRay(const glm::vec3& rayOrigin, const glm::vec3& rayDir, unsigned int shaderId){

    glBindFragDataLocation(shaderId, 0, "fragColor");

    GLuint rayColorUniform = glGetUniformLocation(shaderId, "rayColor");

    // Set ray color (e.g., green)
    glUniform3f(rayColorUniform, 0.0f, 1.0f, 0.0f);

    auto rayEnd = rayOrigin + rayDir * 1000000.0f;
    // std::cout << "Ray End " << rayEnd.x << " " << rayEnd.y << " " << rayEnd.z << std::endl;
    GLfloat lineVertices[] = {
        rayOrigin.x, rayOrigin.y, rayOrigin.z,
        rayEnd.x, rayEnd.y, rayEnd.z
    };
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);

    // Don't forget to delete the VAO and VBO when you're done
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

}

int selectModel(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const std::vector<Model*>& models) {
    int closestModelIndex = -1;
    float closestDistance = std::numeric_limits<float>::max();
    glm::vec3 closestIntersectionPoint;

    for (unsigned int i = 0; i < models.size();i++) {
        const auto meshes = *models.at(i)->getMeshes();
        for (unsigned int j = 0; j < meshes.size();j++) {
            auto mesh = meshes.at(j);
            for (int k = 0; k < mesh.indices.size(); k+=3) {

                unsigned int index0 = mesh.indices.at(k);
                unsigned int index1 = mesh.indices.at(k + 1);
                unsigned int index2 = mesh.indices.at(k + 2);

                glm::vec3 v0 = mesh.vertices.at(index0).Position;
                glm::vec3 v1 = mesh.vertices.at(index1).Position;
                glm::vec3 v2 = mesh.vertices.at(index2).Position;
    
                float distance = std::numeric_limits<float>::max();
                glm::vec2 baryPosition;

                    if ( glm::intersectRayTriangle(rayOrigin, rayDir, v0, v1, v2, baryPosition, distance)) {
                        // Calculate the distance from the camera to the intersection point
                        glm::vec3 intersectionPoint = rayOrigin + rayDir * distance;
                        // std::cout << "Distance " << distance << std::endl;
                        if (distance < closestDistance) {
                            if (distance > 0 && baryPosition.x >= 0 && baryPosition.y >= 0 && (baryPosition.x + baryPosition.y) <= 1) {
                                // Valid intersection
                                std::cout << "Intersection detected!!" << std::endl;
                            }
                            std::cout << "Distance Low!!!" << std::endl;
                            closestDistance = distance;
                            closestModelIndex = i;
                            closestIntersectionPoint = intersectionPoint;
                        }
                    }
            }
        }
    }
        // std::cout << "Selected Model index " << closestModelIndex << std::endl;
        return closestModelIndex;
}

glm::vec3 getNormalizedCoordinateForDepth(double winX, double winY, double screenWidth, double screenHeight, double depth){
    return glm::vec3(
    (2.0 * winX) / screenWidth - 1.0,
    1.0 - (2.0 * winY) / screenHeight,
    1.0f // Adjust depth to range [-1, 1]
);}

void setRay(double winX, double winY, glm::vec3& rayOrigin, glm::vec3& rayDir, glm::mat4 &glmModelView, glm::mat4 &glmProjection, glm::vec3 cameraDirection)
{
    // glm::vec3 ndc = getNormalizedCoordinateForDepth(winX, winY,800,600,0);
    // glm::vec3 rayOriginNDC(ndc.x, ndc.y, 0.0f);
    // glm::vec3 rayEndNDC(ndc.x, ndc.y, 1.0f); 

    // std::cout << "Near plane: " << rayOriginNDC.x << "  " << rayOriginNDC.y << "  " << rayOriginNDC.z << std::endl;
    // std::cout << "Far plane: " << rayEndNDC.x << "  " << rayEndNDC.y << "  " << rayEndNDC.z << std::endl;

    // rayOrigin = unProject(rayOriginNDC, glmModelView, glmProjection, glm::vec4(0.0f, 0.0f, 800, 600));
    // glm::vec3 rayEnd = unProject(rayEndNDC, glmModelView, glmProjection, glm::vec4(0.0f, 0.0f, 800, 600));

    // //Ray direction and length
    // rayDir = glm::vec3(rayEnd.x - rayOrigin.x, rayEnd.y - rayOrigin.y, rayEnd.z - rayOrigin.z);

    float x = (2.0f * winX) / 1280 - 1.0f;
    float y = 1.0f - (2.0f * winY) / 800;
    float z = 1.0f;

    // Normalized device space
    glm::vec3 ray_nds(x, y, z);

    // Clip space
    glm::vec4 ray_clip(ray_nds.x, ray_nds.y, -1.0f, 1.0f);

    // Eye space
    glm::mat4 invProjMat = glm::inverse(glmProjection);
    glm::vec4 ray_eye = invProjMat * ray_clip;
    ray_eye.z = -1.0f;
    ray_eye.w = 0.0f;

    // World space
    glm::mat4 invViewMat = glm::inverse(glmModelView);
    rayOrigin = glm::vec3(invViewMat * ray_eye);

    // Normalize the vector
    rayOrigin = glm::normalize(rayOrigin);

    rayDir = glm::normalize(cameraDirection);

    // glm::vec3 cameraPos = glm::vec3(glm::inverse(glmModelView)[3]);
    // rayDir = glm::normalize(rayOrigin - cameraPos);
}

void handlePicking(double mouseX, double mouseY, const std::vector<Model*>& models, glm::mat4 &view, glm::mat4 &projection, unsigned int rayShader, glm::vec3 cameraDirection) {
    // std::cout << "Mouse Position" << mouseX << " " << mouseY << std::endl;
    glm::vec3 rayOrigin, rayDir;
    setRay(mouseX, mouseY, rayOrigin, rayDir, view, projection, cameraDirection);
    renderRay(rayOrigin, rayDir, rayShader); 
    // std::cout << "Ray direction " << rayDir.x << " " << rayDir.y << " " << rayDir.z << std::endl;
    // std::cout << "Ray origin " << rayOrigin.x << " " << rayOrigin.y << " " << rayOrigin.z << std::endl;
    int selectedModelIndex = selectModel(rayOrigin, rayDir, models);
    std::cout << "Index: " << selectedModelIndex << std::endl;
    // if (selectedModel != nullptr) {
    // }
}
