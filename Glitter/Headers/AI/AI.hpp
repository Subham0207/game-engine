#include <glm/glm.hpp>

namespace Controls{
    class PlayerController;
}
namespace AI
{
    class AI
    {
    public:
        AI(Controls::PlayerController* playerController);
        void onStart();
        void Tick(float deltaTime, glm::vec3 pos);
        void setPath(std::vector<glm::vec3> path);
        void calculatePath(glm::vec3 startingPos, glm::vec3 targetPos);
    private:
        Controls::PlayerController* playerController;

        glm::vec3 targetDirection;
        bool targetDirChoosen;

        float elapsedTime;

        float arrivalRadius;
        std::vector<glm::vec3> path;
        int currentPathIndex;
    };
}