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
        void Tick(float deltaTime);
    private:
        Controls::PlayerController* playerController;

        glm::vec3 targetDirection;
        bool targetDirChoosen;

        float elapsedTime;
    };
}