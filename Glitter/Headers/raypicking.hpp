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

#include <Input.hpp>

#include <glitter.hpp>

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
                                // std::cout << "Intersection Detected " << baryPosition.x << " " << baryPosition.y << std::endl; 
                                closestDistance = distance;
                                closestModelIndex = i;
                                closestIntersectionPoint = intersectionPoint;
                            }
                        }
                    }
            }
        }
    }
        // std::cout << "Selected Model index " << closestModelIndex << std::endl;
        // return -1;
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
    float mouseX = (winX / float(mWidth)) * 2.0f - 1.0f;
    float mouseY = (winY / float(mHeight)) * 2.0f - 1.0f;
    mouseY = -mouseY;  // Invert Y coordinate because OpenGL's origin is at the bottom left

    // Assuming glmProjection is your projection matrix and glmModelView is your model-view matrix
    glm::mat4 invVP = glm::inverse(glmProjection * glmModelView);
    glm::vec4 nearScreenPos = glm::vec4(mouseX, mouseY, -1.0f, 1.0f); // Near plane
    glm::vec4 farScreenPos = glm::vec4(mouseX, mouseY, 1.0f, 1.0f); // Far plane

    // Transform the screen positions to world coordinates
    glm::vec4 nearWorldPos = invVP * nearScreenPos;
    glm::vec4 farWorldPos = invVP * farScreenPos;

    // Normalize w component to get correct x, y, z values
    nearWorldPos /= nearWorldPos.w;
    farWorldPos /= farWorldPos.w;

    // Define the ray origin and direction
    rayOrigin = glm::vec3(nearWorldPos);  
    rayDir = glm::normalize(glm::vec3(farWorldPos - nearWorldPos));

}

void handlePicking(
    double mouseX,
    double mouseY,
    const std::vector<Model*>& models,
    glm::mat4 &view,
    glm::mat4 &projection,
    unsigned int rayShader,
    glm::vec3 &rayOrigin,
    glm::vec3 &rayDir,
    glm::vec3 cameraDirection) {
    if(InputHandler::currentInputHandler->leftClickPressed)
    {
        setRay(mouseX, mouseY, rayOrigin, rayDir, view, projection, cameraDirection);
        // std::cout << "Ray direction " << rayDir.x << " " << rayDir.y << " " << rayDir.z << std::endl;
        // std::cout << "Ray origin " << rayOrigin.x << " " << rayOrigin.y << " " << rayOrigin.z << std::endl;
        int selectedModelIndex = selectModel(rayOrigin, rayDir, models);
        std::cout << "Intersected Model index: " << selectedModelIndex << std::endl;
    }
    renderRay(rayOrigin, rayDir, rayShader); 
}
