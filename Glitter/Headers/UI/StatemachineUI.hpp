#pragma once

namespace UI
{
    class StatemachineUI
    {
        public:
            StatemachineUI();

            void draw();
            void start();
            
            bool showUI;

            private:
                char stateMachineName[256]{};
    };
}