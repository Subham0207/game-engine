# Multi-window Editor + StateMachine windowing (current approach)

This note documents the current state of the **separate windowing** work for the main **Editor** window and the **StateMachine** tool window.

> Goal: Have two independent GLFW windows, each with its own OpenGL context + its own ImGui/ImNodes contexts and backends, so UI/input/state do not leak between windows.

---

## High-level architecture

### 1) `GameWindow` base class
Located at: `Glitter/Headers/Windowing/GameWindow.hpp`

Each top-level window derives from `GameWindow` and implements:

- `init()` – create GLFW window + initialize per-window contexts/backends
- `tick()` – one frame of update/render for that window
- `shutdown()` – destroy per-window backends/contexts and destroy the GLFW window

Each `GameWindow` stores:

- `GLFWwindow* mWindow`
- `ImGuiContext* mImguiContext`
- `ImNodesContext* mImNodesContext`

and provides simple helpers:

- `makeCurrent()` – calls `glfwMakeContextCurrent(mWindow)`
- `setImguiCurrent()` – calls `ImGui::SetCurrentContext(mImguiContext)`

### 2) Window manager loop in `Editor::openEditor()`
Located at: `Glitter/Sources/Editor.cpp`

`Editor::openEditor()` is reduced to:

1. Engine init (create `EngineState`, bootstrap Lua)
2. Create a list of `GameWindow` instances:
   - `EditorWindow`
   - `StateMachineWindow`
3. Run a loop that calls `tick()` on each window until all are closed
4. Per closed window: call `shutdown()` (and erase from the array)
5. Terminate GLFW once after all windows are closed

This makes `openEditor()` behave as a **window spawner / manager** rather than owning a single monolithic render loop.

---

## Window implementations

### `EditorWindow`
Files:
- `Glitter/Headers/Windowing/EditorWindow.hpp`
- `Glitter/Sources/Windowing/EditorWindow.cpp`

This is the main editor window:

- Creates the main GLFW window via `Shared::InitBackEndsWithWindow()`
- Replaces the legacy global ImGui backend initialization with a **dedicated ImGui context** per window
- Owns the editor scene setup (level, skybox, lights, raycast selector, postprocess, etc.)

### `StateMachineWindow`
Files:
- `Glitter/Headers/Windowing/StateMachineWindow.hpp`
- `Glitter/Sources/Windowing/StateMachineWindow.cpp`

This is the separate tool window hosting the state machine editor UI.

#### Split UI layout (vertical split)
The StateMachine window uses a simple ImGui layout split into two vertical parts:

- **Left panel**: reserved for displaying a 3D scene view (currently a placeholder; the code expects a scene texture if exposed).
- **Right panel**: a **NodeGraph** instance rendered embedded into the child region.

The split ratio is controlled by an ImGui slider.

> Note: This separate ImGui UI can be refactored later. It is likely we will eventually add a real 3D scene viewport directly into the StateMachine editor window, or integrate it more cleanly with the renderer pipeline.

---

## Per-window ImGui/ImNodes contexts + isolated backends

### New helper APIs in `Shared`
Files:
- `Glitter/Headers/Helpers/Shared.hpp`
- `Glitter/Sources/Shared.cpp`

Key additions:

- `ImGuiContext* Shared::createImguiContext()`
- `ImNodesContext* Shared::createImNodesContext()`
- `Shared::initImguiBackendForWindow(GLFWwindow*)`
- `Shared::shutdownImguiBackendForWindow()`

### Important backend setting
We use:

- `ImGui_ImplGlfw_InitForOpenGL(window, false)`

This means **ImGui does not install GLFW callbacks**. Callback ownership is handled by the engine’s input system so each window can have isolated routing.

---

## Input isolation (critical fix)

### Problem we solved
When multiple GLFW windows were open:

- One window (often the state machine window) would “hijack” input.
- ImGui IO would be read/updated for the wrong window.

### Current solution
Files:
- `Glitter/Headers/Controls/Input.hpp`
- `Glitter/Sources/Input.cpp`

We introduced `WindowInputUserData` stored per GLFW window using `glfwSetWindowUserPointer`:

```cpp
struct WindowInputUserData
{
    InputHandler* handler;
    InputContext* ctx;
    ImGuiContext* imguiCtx;
    ImNodesContext* imnodesCtx;
};
```

Each input callback:

1. Reads `WindowInputUserData` from the GLFW window that fired the callback
2. Sets the correct ImGui context (`ImGui::SetCurrentContext(ud->imguiCtx)`) so IO is window-correct
3. Forwards the event to ImGui backend functions (`ImGui_ImplGlfw_*Callback`) because `install_callbacks=false`

Forwarded events include:

- Mouse move: `ImGui_ImplGlfw_CursorPosCallback`
- Mouse buttons: `ImGui_ImplGlfw_MouseButtonCallback`
- Scroll: `ImGui_ImplGlfw_ScrollCallback`
- Keyboard: `ImGui_ImplGlfw_KeyCallback`
- Text input: `ImGui_ImplGlfw_CharCallback`
- Focus: `ImGui_ImplGlfw_WindowFocusCallback`

Additionally:

- The windowing layer (`EditorWindow` / `StateMachineWindow`) sets `ud->imguiCtx` and `ud->imnodesCtx` after creating per-window contexts.

This provides independent UI + input behavior between both windows.

---

## `EngineState` window pointers

Previously the engine used a single global window pointer:

- `EngineState::mWindow`

This was replaced with explicit per-window pointers:

- `EngineState::mEditorWindow`
- `EngineState::mStatemachineWindow`

This prevents accidental usage of the wrong window pointer in a multi-window setup.

---

## NodeGraph embedded drawing

`NodeGraph` supports two drawing paths:

- `drawUI()` – creates its own ImGui window (`ImGui::Begin/End`)
- `drawUIEmbedded()` – draws into an already open ImGui region (no Begin/End)

The StateMachine window uses `drawUIEmbedded()` so the node graph can live inside the right-side child panel.

---

## Known follow-ups / cleanup

- The StateMachine window currently uses a placeholder for the left-side scene texture.
  - Expose a renderer output texture (e.g., from `PostProcess`) if the left panel should show live rendering.
- Some ownership is still manual (`new` allocations in window state). Consider converting to `std::unique_ptr`.
- `WindowInputUserData` is currently allocated and never freed. Add cleanup in window shutdown.
- Many includes in `Editor.cpp` are now legacy / unused after refactor.
- Consider consolidating shared renderer assets between windows once the API is stable.

