#include <string>

namespace UI
{
    class AI_UI
    {
        public:
            AI_UI();
            static void start();
            void draw();
            bool showUI;
        private:
            std::string filename;
    };
}