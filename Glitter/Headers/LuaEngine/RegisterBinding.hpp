//
// Created by subha on 19-12-2025.
//

#ifndef GLITTER_REGISTERBINDING_HPP
#define GLITTER_REGISTERBINDING_HPP
class LuaRegistry;

class RegisterBinding
{
public:
    static void RegisterEngineAPI(LuaRegistry& reg);
};


#endif //GLITTER_REGISTERBINDING_HPP