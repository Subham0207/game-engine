//
// Created by subha on 15-02-2026.
//

#ifndef GLITTER_CONTROLLER_HPP
#define GLITTER_CONTROLLER_HPP

namespace Controls
{
    class Controller
    {
    public:
        virtual ~Controller() = default;
        virtual void OnStart() = 0;
        virtual void OnTick(float deltaTime) = 0;
        virtual void OnDestroy() = 0;
    };
}


#endif //GLITTER_CONTROLLER_HPP