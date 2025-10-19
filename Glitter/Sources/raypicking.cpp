#include <Helpers/raypicking.hpp>
#include <Helpers/glitter.hpp>
#include <Controls/Input.hpp>
#include <EngineState.hpp>
#include <limits>

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

bool intersectRayPlane(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDir,
    const glm::vec3& n,          // plane normal (must be normalized)
    const glm::vec3& p,          // a point on plane
    float& tOut,
    glm::vec3& hitOut
) {
    float denom = glm::dot(n, rayDir);
    const float EPS = 1e-6f;
    if (std::fabs(denom) < EPS) return false; // Ray parallel to plane

    float t = glm::dot(p - rayOrigin, n) / denom;
    if (t < 0.0f) return false;               // Intersection is behind the origin

    tOut = t;
    hitOut = rayOrigin + t * rayDir;
    return true;
}

void renderRayWithIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayEnd, unsigned int shaderId){

    glBindFragDataLocation(shaderId, 0, "fragColor");

    GLuint rayColorUniform = glGetUniformLocation(shaderId, "rayColor");

    // Set ray color (e.g., green)
    glUniform3f(rayColorUniform, 0.0f, 1.0f, 0.0f);
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

int selectModel(const glm::vec3& rayOrigin, const glm::vec3& rayDir, glm::vec3& rayEnd, const std::vector<Renderable*>& renderables) {
    int closestModelIndex = -1;
    float closestDistance = std::numeric_limits<float>::max();
    glm::vec3 closestIntersectionPoint;

    for (unsigned int i = 0; i < renderables.size();i++) {
        const auto meshes = *renderables.at(i)->getMeshes();
        for (unsigned int j = 0; j < meshes.size();j++) {
            auto mesh = meshes.at(j);
            for (int k = 0; k < mesh.indices.size(); k+=3) {

                unsigned int index0 = mesh.indices.at(k);
                unsigned int index1 = mesh.indices.at(k + 1);
                unsigned int index2 = mesh.indices.at(k + 2);

                //You see here you have v0,v1,v2 in model's local space :D
                //And that is why you will get intersections assuming the model is at 0,0,0 in world space
                glm::vec3 v0 = mesh.vertices.at(index0).animatedPos;
                glm::vec3 v1 = mesh.vertices.at(index1).animatedPos;
                glm::vec3 v2 = mesh.vertices.at(index2).animatedPos;
    
                float distance = std::numeric_limits<float>::max();
                glm::vec2 baryPosition;

                //convert rayorigin and direction into model's local space
                glm::mat4 inverseModelMatrix = glm::inverse(renderables.at(i)->getModelMatrix());

                // Transform ray to object space
                glm::vec4 homogenousRayOrigin = glm::vec4(rayOrigin, 1.0); // Make ray origin homogeneous
                glm::vec4 homogenousRayDirection = glm::vec4(rayDir, 0.0); // Direction vector, w = 0 to avoid translation

                glm::vec3 localRayOrigin = glm::vec3(inverseModelMatrix * homogenousRayOrigin);
                glm::vec3 localRayDirection = glm::normalize(glm::vec3(inverseModelMatrix * homogenousRayDirection)); // Normalize to correct for any scaling

                    if ( glm::intersectRayTriangle(localRayOrigin, localRayDirection, v0, v1, v2, baryPosition, distance)) {
                        // Calculate the distance from the camera to the intersection point
                        rayEnd = rayOrigin + rayDir * distance;
                        // std::cout << "Distance " << distance << std::endl;
                        if (distance < closestDistance) {
                            if (distance > 0 && baryPosition.x >= 0 && baryPosition.y >= 0 && (baryPosition.x + baryPosition.y) <= 1) {
                                // Valid intersection
                                // std::cout << "Intersection Detected " << baryPosition.x << " " << baryPosition.y << std::endl; 
                                closestDistance = distance;
                                closestModelIndex = i;
                                closestIntersectionPoint = rayEnd;

                                //the intersection point is in local space convert it into world
                                rayEnd = glm::vec3(renderables.at(i)->getModelMatrix() * glm::vec4(rayEnd,1.0));                                
                                State::state->v0 = glm::vec3(renderables.at(i)->getModelMatrix() * glm::vec4(v0,1.0));
                                State::state->v1 = glm::vec3(renderables.at(i)->getModelMatrix() * glm::vec4(v1,1.0));
                                State::state->v2 = glm::vec3(renderables.at(i)->getModelMatrix() * glm::vec4(v2,1.0));
                            }
                        }
                    }
            }
        }
    }
        // std::cout << "Selected ModelType index " << closestModelIndex << std::endl;
        // return -1;
        return closestModelIndex;
}

glm::vec3 getNormalizedCoordinateForDepth(double winX, double winY, double screenWidth, double screenHeight, double depth){
    return glm::vec3(
    (2.0 * winX) / screenWidth - 1.0,
    1.0 - (2.0 * winY) / screenHeight,
    1.0f // Adjust depth to range [-1, 1]
);}

int handlePicking(
    double mouseX,
    double mouseY,
    const std::vector<Renderable*>& renderables,
    glm::mat4 &view,
    glm::mat4 &projection,
    unsigned int rayShader,
    glm::vec3 &rayOrigin,
    glm::vec3 &rayDir,
    glm::vec3 cameraDirection) {
    // -2 means no mouse click; -1 means mouse click but no selection detected; >-1 means index of the selected model
    int selectedModelIndex = -2;
    if(InputHandler::currentInputHandler->leftClickPressed)
    {
        setRay(mouseX, mouseY, rayOrigin, rayDir, view, projection, cameraDirection);
        // std::cout << "Ray direction " << rayDir.x << " " << rayDir.y << " " << rayDir.z << std::endl;
        // std::cout << "Ray origin " << rayOrigin.x << " " << rayOrigin.y << " " << rayOrigin.z << std::endl;
        selectedModelIndex = selectModel(rayOrigin, rayDir, State::state->rayEnd, renderables);
        // std::cout << "Intersected ModelType index: " << selectedRenderableIndex << std::endl;
    }
    // renderRay(rayOrigin, rayDir, rayShader); 
    // renderRayWithIntersection(rayOrigin, State::state->rayEnd, rayShader); 

    // renderRayWithIntersection(State::state->v0, State::state->v1, rayShader); 
    // renderRayWithIntersection(State::state->v1, State::state->v2, rayShader); 
    // renderRayWithIntersection(State::state->v2, State::state->v0, rayShader);

    return selectedModelIndex;
}
