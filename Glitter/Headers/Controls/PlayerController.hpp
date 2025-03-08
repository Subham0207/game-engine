#pragma once

namespace Controls
{
    class PlayerController{        
        public:
        PlayerController()
        {}
            bool isWalking = false;
            bool isRunning = false;
            bool isJumping = false;

        void setWalking()
        {
            isWalking = true;
            isRunning = false;
            isJumping = false;
        }

        void setRunning()
        {
            isWalking = false;
            isRunning = true;
            isJumping = false;
        }

        void setJumping()
        {
            isWalking = false;
            isRunning = false;
            isJumping = true;            
        }

        void setIdleing()
        {
            isWalking = false;
            isRunning = false;
            isJumping = false;            
        }
    };
}