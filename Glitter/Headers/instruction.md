# Multi-window Editor + StateMachine windowing (current approach)

This note documents the current state of the **separate windowing** work for the main **Editor** window and the **StateMachine** tool window.

> Goal: Have two independent GLFW windows, each with its own OpenGL context + its own ImGui/ImNodes contexts and backends, so UI/input/state do not leak between windows.

---

## High-level architecture

### 1) `GameWindow` base class
Located at: `Glitter/Headers/Windowing/GameWindow.hpp`

Each top-level window derives from `GameWindow` and implements:

- `init()` – create GLFW window + initialize per-window contexts/backends
- `tickImpl()` – one frame of update/render for that window (implemented by the derived window)
- `shutdown()` – destroy per-window backends/contexts and destroy the GLFW window

`GameWindow::tick()` is a **non-virtual wrapper** that:

1) Computes per-window `deltaTime` and clamps it to a max (currently `0.1f`) to avoid physics explosions when a window regains focus after being paused/unfocused.
2) Writes `EngineState::state->deltaTime` for legacy systems that still read a global delta time.
3) Calls the derived window’s `tickImpl()`.

This removes the need for using a global `EngineState::lastFrame`.
- `shutdown()` – destroy per-window backends/contexts and destroy the GLFW window

Each `GameWindow` stores (common window “plumbing”):

- `GLFWwindow* mWindow`
- `ImGuiContext* mImguiContext`
- `ImNodesContext* mImNodesContext`

and also provides shared per-window systems:

- `std::unique_ptr<EventQueue> mQueue`
- `std::unique_ptr<InputContext> mInputCtx`
- `std::unique_ptr<InputHandler> mInputHandler` (optional per derived window)
- `std::unique_ptr<FlyCam> mEditorCamera` (per-window default camera)

The helper `initCommonInput()` allocates `EventQueue` + `InputContext` and wires `InputContext::queue`.

> Note: `GameWindow` currently creates a `FlyCam` by default so every window can construct an `InputHandler` even if `EngineState` is not initialized (ex: `ProjectManagerWindow` exe). Later we can refactor this so windows that will never have a 3D scene (like Project Manager) don’t need a camera at all.

and provides simple helpers:

- `makeCurrent()` – calls `glfwMakeContextCurrent(mWindow)`
- `setImguiCurrent()` – calls `ImGui::SetCurrentContext(mImguiContext)`

### 2) Window manager loop in `Editor::openEditor()`
Located at: `Glitter/Sources/Editor.cpp`

`Editor::openEditor()` is reduced to:

1. Engine init (create `EngineState`, bootstrap Lua)
2. Create a list of `GameWindow` instances:
   - `EditorWindow`
   - (Tool windows like `StateMachineWindow` are spawned on-demand)
3. Run a loop that picks the **focused** window as the active window, switches GL + ImGui contexts when focus changes, and calls `tick()` only on that active window.
4. Per closed window: call `shutdown()` (and erase from the array)
5. Terminate GLFW once after all windows are closed

This makes `openEditor()` behave as a **window spawner / manager** rather than owning a single monolithic render loop.

#### On-demand tool window spawning (current)

The `Editor` loop contains a reusable helper that can spawn/focus windows based on a request flag:

```cpp
template<typename TWindow, typename... TArgs>
void spawnOrFocusWindowOnRequest(
    std::vector<std::unique_ptr<GameWindow>>& windows,
    bool& requestToOpenWindow,
    GLFWwindow*& outGlfwHandle,
    TArgs&&... args);
```

The helper:

- Consumes `requestToOpenWindow` (sets it back to `false`)
- If an instance already exists, focuses it
- Otherwise creates `TWindow`, appends it to the `windows` array, and calls `init()`

This is how `StateMachineWindow` is opened from the UI without creating it at editor startup.

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

#### Window open requests (UI -> Editor)

`EditorWindow` exposes a small request struct:

```cpp
struct WindowRequests
{
    bool openStateMachineWindow = false;
};
```

This struct is meant to be written by UI widgets (ex: Outliner button click), and then consumed by the window manager loop in `Editor.cpp`.

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

## Project Manager window (separate executable)

The Project Manager is a separate executable (`projectMangerMain.cpp`) and now uses the same `GameWindow` approach via:

- `Glitter/Headers/Windowing/ProjectManagerWindow.hpp`
- `Glitter/Sources/Windowing/ProjectManagerWindow.cpp`

Instead of duplicating window loops, `ProjectManagerWindow` provides:

- `int StartWindow()` – calls `init()`, runs a `while(!shouldClose())` loop (`glfwPollEvents()` + `tick()`), then `shutdown()`.

This keeps the same backend isolation and input routing model as the Editor/StateMachine windows.

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

## UI triggers for tool windows (StateMachineWindow)

Currently, the Outliner has a "Create new statmachine" button. Instead of directly creating a GLFW window (or running another loop), it:

1. Sets the injected request flag: `EditorWindow::WindowRequests::openStateMachineWindow = true`
2. The `Editor::openEditor()` window manager sees the request and calls `spawnOrFocusWindowOnRequest<StateMachineWindow>(...)`

This means the Outliner does not own the `windows` array and does not directly create/destroy `GameWindow` instances.

---

## Raycast selection input (no globals)

`Debug::Raycast::HandleSelection` was updated to avoid relying on any global/static input pointer. It now takes an explicit `InputHandler*`:

```cpp
void HandleSelection(Outliner* outliner, Camera* activeCamera, const Level* level, InputHandler* input);
```

`EditorWindow` passes its own per-window input handler when calling selection.

---

## Passing Input/Event systems into `Level` + `Character`

To reduce reliance on hidden globals (and to make multi-window input more deterministic), `Level` and `Character` now hold **non-owning pointers** to the window-local input/event systems.

### `Level` owns pointers to:

- `InputHandler*` (the InputHandler created by the owning `GameWindow`)
- `EventQueue*` (the per-window queue from `GameWindow::mQueue`)
- `EventBus*` (currently the global engine bus: `&EngineState::state->bus`)

These are set by `EditorWindow` after it creates the input handler:

- `lvl->setInputHandler(mInputHandler.get());`
- `lvl->setEventQueue(mQueue.get());`
- `lvl->setEventBus(&EngineState::state->bus);`

### Character injection during spawn

`Level::spawnCharacter(...)` injects these same pointers into every newly created character:

- `character->setInputHandler(level->inputHandler);`
- `character->setEventQueue(level->eventQueue);`
- `character->setEventBus(level->eventBus);`

This enables character/controller code to:

- read input from the **correct window** (via the injected `InputHandler*`)
- publish or consume events via the injected `EventQueue*` and `EventBus*`

> Note: right now `EventBus` is still sourced from `EngineState::state->bus`, but passing it explicitly makes dependencies visible and will allow replacing it later with a per-window bus or a world/scene bus if needed.

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

- **Important refactor (decoupling)**: opening tool windows should move to the existing **`EventQueue`** in `GameWindow`.
  - Right now, Outliner is still coupled to `EditorWindow` via an injected pointer to a request flag.
  - A better design is: Outliner publishes an event (ex: `OpenWindow(StateMachine)`), and the Editor/window-manager (or a central dispatcher) consumes it and spawns windows.
  - This will remove the last direct coupling between UI widgets and window-management objects.

- `GameWindow` currently creates a per-window `FlyCam` so even UI-only windows can install an `InputHandler` (needed because ImGui uses `install_callbacks=false`). We can later create a UI-only input layer or allow windows to opt-out of camera creation.

---

# PostProcess FBO + fullscreen pass (resize-safe)

This section documents the design for the **offscreen framebuffer (FBO) + postprocess fullscreen triangle** pipeline.

## The symptom we fixed

When rendering the 3D scene into an FBO texture and then drawing that texture to the screen with a fullscreen triangle + postprocess shader, we observed:

- **Black strips/bars** at the top/right when switching to fullscreen.
- Black strips when snapping/resizing the window (e.g. docking to one side of a monitor).

These artifacts are almost always caused by a mismatch between:

1) The **default framebuffer size** (GLFW *framebuffer* size in pixels)
2) The **FBO attachment sizes** (screen color texture + depth/stencil RBO)
3) The **current OpenGL viewport** when rendering into each target

OpenGL does *not* automatically update viewport state when switching framebuffers, and an FBO’s attachments do *not* automatically resize when the window changes size.

## Current implementation

Files:
- `Glitter/Headers/RenderPipeline/PostProcess.hpp`
- `Glitter/Sources/PostProcess.cpp`

### Key rules

**Rule A: Always set viewport for the target you are rendering into**

- When rendering the scene into the postprocess FBO: `glViewport(0,0,fboW,fboH)`
- When rendering the fullscreen triangle to the default framebuffer: `glViewport(0,0,winFbW,winFbH)`

**Rule B: Resize the FBO attachments when the window framebuffer size changes**

`PostProcess` stores cached attachment dimensions:

- `mFbWidth`, `mFbHeight`

and exposes:

- `PostProcess::resize(int fbWidth, int fbHeight)`

Internally, resize reallocates:

- the HDR color attachment (`screenTexture`) via `glTexImage2D`
- the depth/stencil attachment (`rbo`) via `glRenderbufferStorage`

and re-validates completeness with `glCheckFramebufferStatus`.

**Rule C: Protect fullscreen passes from leftover state**

At minimum, the postprocess fullscreen draw disables scissor:

- `glDisable(GL_SCISSOR_TEST)`

because scissor can clip the fullscreen triangle and look like “black bars”.

## Camera aspect ratio: second-order issue

After making the FBO + viewport resize-safe, we also saw a frustum/aspect mismatch that *looked like clipping* when the window got smaller.

Root cause:

- `Camera::tick()` computes perspective aspect ratio from global `mWidth/mHeight`.
- In a multi-window engine, relying on global width/height is fragile.

Current mitigation (EditorWindow):

- In `EditorWindow::tickImpl()` we query the real per-window framebuffer size using `glfwGetFramebufferSize(mWindow, &fbW, &fbH)` and update:
  - `mWidth = fbW; mHeight = fbH;`
  - `mPostProcess->resize(fbW, fbH)`
  - then call `activeCamera->tick()`

This keeps the camera projection matrix consistent with the current render target.

---

# Upcoming refactor: reusable 3D scene rendering for any window

We plan to render a real 3D scene inside `StateMachineWindow` (and potentially other tool windows). To avoid duplicating a large amount of custom rendering code per window, we should refactor the rendering pipeline around a reusable “scene view” concept.

## Problem

Today, the scene rendering logic is effectively embedded in `EditorWindow` (level tick, lighting, shadow, postprocess, skybox, etc.). Tool windows will need the same features (or a subset) without copying editor-specific behavior.

## Direction

Introduce a reusable component (names TBD):

- `SceneView` / `SceneRenderer` / `ViewportRenderer`

Responsibilities:

1) Own a resizeable render target (FBO + color texture + depth/stencil)
2) Expose a texture ID for ImGui embedding (or direct blit to window)
3) Provide explicit `resize(fbW, fbH)` and enforce viewport correctness
4) Render a set of `Renderable`s with a provided `Camera*` and `Lights*`

Then each window does only:

- Determine its desired viewport region size (full window framebuffer, or an ImGui child region)
- Call `sceneView.resize(w,h)`
- Call `sceneView.render(...)`
- Display the resulting texture (either postprocess-to-screen OR ImGui::Image)

## Notes

- The “authoritative size” should be the **framebuffer pixel size** (GLFW framebuffer size).
- If we render into an ImGui panel, we must compute the panel size in pixels and resize the scene view accordingly.
- Avoid global `mWidth/mHeight` for camera projection. Prefer per-camera or per-scene-view dimensions.


