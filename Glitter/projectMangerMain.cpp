// Local Headers
#include <cstdlib>
#include "ProjectManagerHandler.hpp"

int main(int argc, char * argv[])
{
    auto projectmanager = new ProjectManagerHandler();
    projectmanager->startProjectManager();
    return EXIT_SUCCESS;
}