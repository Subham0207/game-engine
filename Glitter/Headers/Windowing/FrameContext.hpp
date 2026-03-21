#pragma once

// Per-frame context that describes the current render target size for a given GameWindow.
// Naming uses "screenWidth/screenHeight" to match the engine's terminology (window == framebuffer).
struct FrameContext
{
    int screenWidth = 0;
    int screenHeight = 0;
    float aspect = 1.0f;
};

