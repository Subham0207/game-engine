//
// Created by subha on 21-12-2025.
//

#ifndef GLITTER_CLIENTHANDLER_HPP
#define GLITTER_CLIENTHANDLER_HPP
#include "Input.hpp"


class ClientHandler
{
public:
    InputHandler* inputHandler{};
    static ClientHandler* clientHandler;
    ClientHandler();
};


#endif //GLITTER_CLIENTHANDLER_HPP