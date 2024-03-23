#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "model.hpp"
#include <vector>

// remove max macro from windows.h and include max from limits because they are conflicting
#ifdef max
#undef max
#endif
#include <limits>

using namespace glm;

class RayPick{
public:

RayPick(std::vector<Model*> objects){
    this->objects = objects;
}

bool rayBox(double *low, double *high, double *origin, double *destination)
{
    double direction[3];
    // for each slab (X plane, Y plane, Z plane)
    double tnear = std::numeric_limits<double>::min();
    double tfar = std::numeric_limits<double>::max();
    for (int i = 0; i < 3; i++)
    {
        direction[i] = destination[i] - origin[i];
        if (direction[i] == 0) // parallel
        {
            if (origin[i] < low[i] || origin[i] > high[i])
                return false;
        }
        else
        {
            double t1 = (low[i] - origin[i]) / direction[i];
            double t2 = (high[i] - origin[i]) / direction[i];
            if (t1 > t2) // swap t1, t2
            {
                double tmp = t1;
                t1 = t2;
                t2 = tmp;
            }
            if (t1 > tnear) // want largest tnear
                tnear = t1;
            if (t2 < tfar) // want smallest tfar
                tfar = t2;
            if (tnear > tfar) // box is missed
                return false;
            if (tfar < 0) // box behind ray origin
                return false;
        }
    }
    return true;
}
void setRay(int mouseX, int mouseY)
{
    int viewport[4];
    double matModelView[16], matProjection[16];

    glGetDoublev(GL_MODELVIEW_MATRIX, matModelView);
    glGetDoublev(GL_PROJECTION_MATRIX, matProjection);
    glGetIntegerv(GL_VIEWPORT, viewport);
    // window pos of mouse, Y is inverted on windows
    double winX = (double)mouseX;
    // double winY = viewport[3] - (double)mouseY;
    double winY = (double)mouseY;
    //Change 800 and 600 later to screen coordinates later

    // get point on the 'near' plan
    glm::mat4 glmModelView = glm::make_mat4(matModelView);
    glm::mat4 glmProjection = glm::make_mat4(matProjection);

    glm::vec3 nearplane = getNormalizedCoordinateForDepth(winX, winY,800,600,0);
    glm::vec3 farplane = getNormalizedCoordinateForDepth(winX, winY,800,600,1);

    // std::cout << "Near plane: " << nearplane.x << "  " << nearplane.y << "  " << nearplane.z << std::endl;
    // std::cout << "Far plane: " << farplane.x << "  " << farplane.y << "  " << farplane.z << std::endl;

    m_start = unProject(nearplane, glmModelView, glmProjection, glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]));
    //get point on the 'far' plan
    m_end = unProject(farplane, glmModelView, glmProjection, glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]));

    // std::cout << "Ray start: " << m_start.x << "  " << m_start.y << "  " << m_start.z << std::endl;
    // std::cout << "Ray end: " << m_end.x << "  " << m_end.y << "  " << m_end.z << std::endl;


    //Ray direction and length
    double rayDirX = farplane.x - nearplane.x;
    double rayDirY = farplane.y - nearplane.y;
    double rayDirZ = farplane.z - nearplane.z;

    std::cout << "Ray direction " << rayDirX << " " << rayDirY << " " << rayDirZ << std::endl;

    // Normalize the ray direction
    double length = sqrt(rayDirX * rayDirX + rayDirY * rayDirY + rayDirZ * rayDirZ);
    rayDirX /= length;
    rayDirY /= length;
    rayDirZ /= length;

    std::cout << "lenght of ray " << length << std::endl;
}

void rayTestObjects(int mouseX, int mouseY)
{
    setRay(mouseX, mouseY);

    double ray_origin[] = {m_start.x, m_start.y, m_start.z};
    double ray_destination[] = {m_end.x, m_end.y, m_end.z};

    double minDistance = std::numeric_limits<double>::max();
    int indexMin = -1;
    selectedLight = -1;
    for (int i = 0; i < objects.size(); i++)
    {
        Model* o = objects.at(i);
        double o_low[] = {o->getX() + SIZE_BOX / 2 * o->getScaleX(), o->getY() + SIZE_BOX / 2 * o->getScaleY(), o->getZ() + SIZE_BOX / 2 * o->getScaleY()};
        double o_high[] = {o->getX() - SIZE_BOX / 2 * o->getScaleX(), o->getY() - SIZE_BOX / 2 * o->getScaleY(), o->getZ() - SIZE_BOX / 2 * o->getScaleY()};

        if (rayBox(o_low, o_high, ray_origin, ray_destination))
        {
            double distance = sqrt(pow(o->getX() - m_start.x, 2) + pow(o->getY() - m_start.y, 2) + pow(o->getZ() - m_start.z, 2));
            if (distance < minDistance)
            {
                minDistance = distance;
                indexMin = i;
                selectedLight = -1;
            }
        }
    }

    // test if light is selected
    // it should be ray sphere but it's ray box now
    // double l0_low[] = {light_pos0[0] + 3, light_pos0[1] + 3, light_pos0[2] + 3};
    // double l0_high[] = {light_pos0[0] - 3, light_pos0[1] - 3, light_pos0[2] - 3};
    // if (rayBox(l0_low, l0_high, ray_origin, ray_destination))
    // {
    //     double distance = sqrt(pow(light_pos0[0] - m_start.x, 2) + pow(light_pos0[1] - m_start.y, 2) + pow(light_pos0[2] - m_start.z, 2));
    //     if (distance < minDistance)
    //     {
    //         minDistance = distance;
    //         indexMin = -1;
    //         selectedLight = 0;
    //     }
    // }

    // double l1_low[] = {light_pos1[0] + 3, light_pos1[1] + 3, light_pos1[2] + 3};
    // double l1_high[] = {light_pos1[0] - 3, light_pos1[1] - 3, light_pos1[2] - 3};
    // if (rayBox(l1_low, l1_high, ray_origin, ray_destination))
    // {
    //     double distance = sqrt(pow(light_pos1[0] - m_start.x, 2) + pow(light_pos1[1] - m_start.y, 2) + pow(light_pos1[2] - m_start.z, 2));
    //     if (distance < minDistance)
    //     {
    //         minDistance = distance;
    //         indexMin = -1;
    //         selectedLight = 1;
    //     }
    // }
    selectedObj = indexMin;
    selectedAxis = -1;
}

int returnIndexOfSelectedObject(){
    return selectedObj;
}

private:
    int selectedObj = -1;
    int selectedAxis = -1;

    float light_pos1[4] = {};
    float light_pos0[4] = {};

    int selectedLight = -1;
    std::vector<Model*> objects;
    const double SIZE_BOX = 60.0;

    glm::vec3 m_start;
    glm::vec3 m_end;

    glm::vec3 getNormalizedCoordinateForDepth(double winX, double winY, double screenWidth, double screenHeight, double depth){
    //     return glm::vec3(
    //     (2.0 * winX) / screenWidth - 1.0,
    //     1.0 - (2.0 * winY) / screenHeight,
    //     2.0 * depth - 1.0 // Adjust depth to range [-1, 1]
    // );
        return glm::vec3(
        (2.0 * (winX + 0.5)) / screenWidth - 1.0,
        1.0 - 2.0 * (winY + 0.5) / screenHeight,
        2.0 * depth - 1.0 // Adjust depth to range [-1, 1]
    );
    }
};