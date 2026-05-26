# AGENTS

This file documents project knowledge and coding conventions for AI coding assistants.

## Scope and Precedence
- A root-level `AGENTS.md` applies to the whole repository.
- You can also place `AGENTS.md` files inside subfolders for local rules.
- Precedence rule: the nearest `AGENTS.md` to the edited file wins for that subtree.

## Testing Conventions (GoogleTest)
- Test source file naming must follow: `<SourceFile>.test.cpp`.
  - Example: tests for `Glitter/Sources/NodeGraph.cpp` live in `Tests/NodeGraph/NodeGraph.test.cpp`.
- Test case names must start with `should...`.
  - Example: `TEST(NodeGraphImNodesScope, shouldQueryHoverApisAfterEndNodeEditor)`.
- New tests should validate the target source file behavior/contract directly.

## Practical Rule for Agents
- Before adding tests, check this file and follow the naming rules exactly.

## Session Handoff Notes (2026-05-10)
- Primary workspace: `E:\OpenGL\game-engine`.
- Environment: Windows with `powershell.exe` (Windows PowerShell v5.1).
- If terminal commands are suggested, keep commands copyable and on separate lines.
- Use fenced code blocks for run/setup commands.
- When uncertain about build/runtime status, avoid definitive claims unless verified.
- If adding tests, continue to follow the naming rules in this file.

## Build Command Convention
- Always build the test target with this exact command:
  `cmake.exe --build "E:\OpenGL\game-engine\cmake-build-debug-visual-studio" --target Test --config Debug`

## Session Handoff Notes (2026-05-25)
- FlowScript compile path now has regression coverage for banner behavior, compile diagnostics capture, and diagnostics reset across recompile.
- `LuaTranspiler::Transpile` now has regression tests for comparison expressions in return statements, including deserialized graph inputs.
- FlowScript AST traversal was hardened so statement input expressions are resolved before exec-chain traversal decisions, and exec passthrough nodes continue traversal to downstream statements.
- FlowScript JSON deserialization now restores serialized FIELD attribute values (for example Boolean/Integer literal node outputs) by attribute id.
- FlowScript node position persistence on load was fixed by leaving deserialized nodes as `positionSet=false` so first draw applies saved screen positions.
- State machine JSON load diagnostics were improved: deserialize converts string paths to `std::filesystem::path`, opens in binary mode, and logs absolute path + exists/file-type checks when open fails.

## Session Handoff Notes (2026-05-26)
- NodeGraph delete behavior now removes selected nodes on `Delete` (not only selected links).
- Delete-node behavior applies to both regular node-graph nodes and state-machine nodes.
- Deleting regular node-graph nodes now also removes attached node links by matching deleted node attribute ids (inputs/outputs/exec pins).
- Deleting state-machine nodes now also removes attached transitions (`fromNodeId`/`toNodeId`).
- Delete hotkey now checks `!ImGui::GetIO().WantTextInput` to avoid deleting graph elements while typing in text fields.
- Regression coverage added in `Tests/NodeGraph/NodeGraph.test.cpp` for regular-node and state-machine node deletion cleanup behavior.

## Session Handoff Notes (2026-05-26, Editor Architecture Snapshot)
- Entry point is `Glitter/editorMain.cpp`, which creates `Editor` and calls `Editor::openEditor()`.
- `Editor::openEditor()` (in `Glitter/Sources/Editor.cpp`) initializes `EngineState`, Lua registry, then runs a multi-window loop over `std::vector<std::unique_ptr<GameWindow>>`.
- `GameWindow` (in `Glitter/Headers/Windowing/GameWindow.hpp`) is the shared base for standalone windows; it owns per-window input/event queue, camera, ImGui + ImNodes contexts, and frame timing.
- The active editor scene is hosted in `EditorWindow` (`Glitter/Sources/Windowing/EditorWindow.cpp`), which sets up level loading, scene viewport, lights, outliner, asset browser, and play/editor camera switching.
- `StateMachineWindow` is a separate `GameWindow` implementation with its own preview level, camera/input flow, and `StateMachineGraph` / FlowScript UI.
- Input callbacks are routed through `WindowInputUserData` so each GLFW window can select the correct ImGui/ImNodes context and input handler.

## Session Handoff Notes (2026-05-26, RmlUi HUD Integration)
- RmlUi is integrated in CMake via `FetchContent` (`mikke89/RmlUi`, tag `6.0`) and linked to `GlitterLib` with backend target `rmlui_backend_GLFW_GL3`.
- Current integration sets `RMLUI_FONT_ENGINE=none` to keep setup dependency-light on this workspace (no detected FreeType package), so HUD styling is available immediately while text/font rendering can be enabled later by adding FreeType and switching back to the freetype font engine.
- New HUD runtime wrapper: `Glitter/Headers/UI/Hud/HudSystem.hpp` and `Glitter/Sources/UI/Hud/HudSystem.cpp`.
- `EditorWindow` now initializes HUD from `EngineAssets/UI/hud.rml`, renders it each frame during play mode, and shuts it down with the window lifecycle.
- GLFW input callbacks in `Input.cpp` now forward keyboard/mouse/scroll/char events to RmlUi context (when present) before continuing normal editor input handling.
- Framebuffer resize callback in `Shared.cpp` now forwards new dimensions to RmlUi so HUD layout tracks viewport size changes.

## Session Handoff Notes (2026-05-26, GlitterLib Packaging)
- To support downstream projects that only link `GlitterLib`, RmlUi GLFW/GL3 backend sources are compiled directly into `GlitterLib`.
- `GlitterLib` now links RmlUi transitively (`RmlUi::RmlUi`) so consumers do not need to add separate RmlUi linkage in their project CMake files.
- Glitter package export now includes RmlUi targets (`rmlui`, `rmlui_core`, `rmlui_debugger`) in `GlitterTargets.cmake`, enabling installed-package consumers to resolve transitive links.

