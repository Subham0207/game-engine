//
// Created by subha on 13-03-2026.
//

#include "../Headers/Helpers/GetExecutablePath.hpp"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

fs::path GetExecutablePath::getExecutableDir()
{
#ifdef _WIN32
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return fs::path(path).parent_path();
#else
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return fs::path(std::string(result, (count > 0) ? count : 0)).parent_path();
#endif
}