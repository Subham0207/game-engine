#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Helpers{
    class Window3D
    {
        public:
        Window3D()
        {
            auto mWindow = glfwCreateWindow(width, height, "OpenGL", nullptr, nullptr);
        }

        private:
         GLFWwindow* window;
         int width;
         int height;

    };
}