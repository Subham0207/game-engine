#pragma once

#include "Windowing/GameWindow.hpp"

#include <filesystem>
#include <memory>

namespace ProjectManagerUI { class ProjectManager; }

// A standalone GameWindow hosting the Project Manager UI.
class ProjectManagerWindow final : public GameWindow
{
public:
	ProjectManagerWindow();
	~ProjectManagerWindow() override;

	ProjectManagerWindow(const ProjectManagerWindow&) = delete;
	ProjectManagerWindow& operator=(const ProjectManagerWindow&) = delete;
	ProjectManagerWindow(ProjectManagerWindow&&) noexcept = delete;
	ProjectManagerWindow& operator=(ProjectManagerWindow&&) noexcept = delete;

	void init() override;
	void tickImpl() override;
	void shutdown() override;

	// Standalone entry for the Project Manager executable.
	// This wraps init() + a while-loop calling glfwPollEvents() + GameWindow::tick().
	int StartWindow();

private:
	std::filesystem::path mPath;
	std::unique_ptr<ProjectManagerUI::ProjectManager> mProjectManagerUI;
};



