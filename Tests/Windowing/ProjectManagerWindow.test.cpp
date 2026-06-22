#include <gtest/gtest.h>

#include <GLFW/glfw3.h>

#include "Controls/Input.hpp"
#include "Windowing/ProjectManagerWindow.hpp"

TEST(ProjectManagerWindowInitRegression, shouldAvoidDuplicateImguiBackendInitInProjectManagerWindow)
{
    // What/Why: The startup regression was a runtime WndProc recursion crash.
    // This test exercises init + event polling + frame ticks + shutdown directly.
    ProjectManagerWindow window;
    ASSERT_NO_FATAL_FAILURE(window.init());
    ASSERT_NE(window.window(), nullptr);

    auto* userData = static_cast<WindowInputUserData*>(glfwGetWindowUserPointer(window.window()));
    ASSERT_NE(userData, nullptr);
    ASSERT_NE(userData->imguiCtx, nullptr);
    ASSERT_NE(userData->imnodesCtx, nullptr);

    for (int frame = 0; frame < 10; ++frame)
    {
        ASSERT_FALSE(window.shouldClose());
        glfwPollEvents();
        ASSERT_NO_FATAL_FAILURE(window.tick());
    }

    ASSERT_NO_FATAL_FAILURE(window.shutdown());
    EXPECT_TRUE(window.shouldClose());

    // Keep test process cleanup explicit when creating standalone GLFW windows in tests.
    glfwTerminate();
}

