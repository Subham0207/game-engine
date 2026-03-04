//
// Created by subha on 04-03-2026.
//

#ifndef GLITTER_PROFILER_H
#define GLITTER_PROFILER_H

#pragma once

#ifdef TRACY_ENABLE
    #include <tracy/Tracy.hpp>
    #include <tracy/TracyOpenGL.hpp>
#else
    // Dummy macros so the code still compiles without the Tracy headers
    #define ZoneScoped
    #define FrameMark
    #define TracyGpuCollect
    #define TracyGpuContext
    #define ZoneScopedN(name)
    #define TracyGpuZone(name)
    // ... add others as needed
#endif

#endif //GLITTER_PROFILER_H