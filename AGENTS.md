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
- Always build with this exact command:
  `cmake.exe --build "E:\OpenGL\game-engine\cmake-build-debug-visual-studio" --target ALL_BUILD --config Debug`

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
- Current integration sets `RMLUI_FONT_ENGINE=freetype` and uses vendored `freetype` (`VER-2-13-3`) for consistent engine/consumer behavior.
- FreeType is configured with `FT_DISABLE_BZIP2=ON` to avoid unresolved BZ2 symbols in full-link targets.
- New HUD runtime wrapper: `Glitter/Headers/UI/Hud/HudSystem.hpp` and `Glitter/Sources/UI/Hud/HudSystem.cpp`.
- `EditorWindow` seeds default `hud.rml`/`hud.rcss` into `<Project>/Assets/HUD` when missing.
- `EditorWindow` renders HUD before ImGui draw data so editor UI remains clickable on top.
- `HudSystem` no longer hardcodes gameplay HUD IDs/styles and no longer traverses DOM to force `div` display.
- HUD layout defaults should be declared in RCSS (for example `div { display: block; }`) rather than C++.
- GLFW input callbacks in `Input.cpp` forward keyboard/mouse/scroll/char events to RmlUi, but HUD callbacks are non-blocking so ImGui input still works.
- Framebuffer resize callback in `Shared.cpp` now forwards new dimensions to RmlUi so HUD layout tracks viewport size changes.

## Session Handoff Notes (2026-05-27, Event-Driven Dynamic HUD)
- `EventType` now includes `HUDUpdate` and `ActivateHUD` (`Glitter/Headers/Event/Event.hpp`).
- `HUDUpdateEvent` supports `SetStyle` and `SetText` operations with payload: `elementId`, `property`, `value`.
- `ActivateHUDEvent` now uses dynamic string keys (`hudKey`) instead of hardcoded enum layers.
- HUD events are queued through `EventQueue` and then dispatched by `GameWindow::tick()` via `EventBus`.
- `HudSystem` stores the window `EventBus` pointer, subscribes to HUD events, and applies updates/activations without requiring HUD references across gameplay code.
- `HudSystem::discoverHudDocuments()` scans `<Project>/Assets/HUD` for `*.rml`; each file stem is the HUD key and matching `*.rcss` with same stem is associated when present.
- Default startup activation uses key `"hud"` (from `hud.rml`) via queued `ActivateHUDEvent`.

## Session Handoff Notes (2026-05-26, GlitterLib Packaging)
- To support downstream projects that only link `GlitterLib`, RmlUi GLFW/GL3 backend sources are compiled directly into `GlitterLib`.
- `GlitterLib` now links RmlUi transitively (`RmlUi::RmlUi`) so consumers do not need to add separate RmlUi linkage in their project CMake files.
- Glitter package export now includes RmlUi targets (`rmlui`, `rmlui_core`, `rmlui_debugger`) in `GlitterTargets.cmake`, enabling installed-package consumers to resolve transitive links.
- Glitter package export/install includes `freetype` when bundled, and generated config handles `Freetype` dependency resolution for consumers.

## HUD Usage (Project Side)
- Author HUD files in your active project under `Assets/HUD`.
- Each `*.rml` is a HUD screen; key is file stem (for example `player_hud.rml` -> key `player_hud`).
- Use same-stem `*.rcss` files for per-screen styles (for example `player_hud.rcss`).
- If project HUD files do not exist, editor startup seeds them from engine defaults in `EngineAssets/UI`.
- HUD is rendered in play mode by `UI::Hud::HudSystem` (`Glitter/Sources/UI/Hud/HudSystem.cpp`).
- Keep HUD styling/data in RML/RCSS; avoid relying on hardcoded element IDs in engine code.
- Use queued events to control HUD from gameplay code:
  - `ActivateHUDEvent("player_hud")` to switch active HUD.
  - `HUDUpdateEvent(SetStyle, "stamina-bar", "width", "84%")` to update styles.
  - `HUDUpdateEvent(SetText, "stamina-text", "", "84 / 100")` to update text.
- Font loading uses project `Assets/Roboto` first (if present), then engine `EngineAssets/Roboto` fallback.
- Input events are shared with ImGui; HUD should not block editor UI clicks by default.

## Session Handoff Notes (2026-05-28, Jolt Character Collision + Physics Debug Refactor)
- Character-vs-character collision for `JPH::CharacterVirtual` is now enabled through a shared `JPH::CharacterVsCharacterCollisionSimple` registry in `Glitter/Sources/capsule.cpp`.
- Each `Physics::Capsule` registers/unregisters its `CharacterVirtual` with that registry during create/reinit/destroy.
- `MyContactListener` now tracks per-frame character contacts (other character ids), not only ground landing events.
- `Physics::Capsule` exposes query helpers: `hasCharacterCollision()`, `getCollidingCharacterIds()`, and `getCharacterId()`.
- `Character` exposes higher-level helpers: `hasCharacterCapsuleCollision()`, `getCollidingCharacterCapsuleIds()`, `getCollidingCharacters()`, `getCapsuleCharacterId()`, and static `getCharacterByCapsuleId(...)`.
- Character lookup map (`capsule id -> Character*`) is maintained in `Character` and synced on create/recreate/draw/destroy.

## Session Handoff Notes (2026-05-28, Jolt Debug Rendering Integration)
- Jolt debug rendering is integrated through `Physics::OpenGLJoltDebugRenderer` (`Glitter/Headers/Physics/OpenGLJoltDebugRenderer.hpp`, `Glitter/Sources/Physics/OpenGLJoltDebugRenderer.cpp`).
- `PhysicsSystemWrapper` owns the debug renderer and exposes `DrawDebugBodies(...)`.
- Editor uses `getUIState().renderPhysicsDebug` toggle to control physics debug visibility.
- Physics debug is intentionally disabled while `EngineState::state->isPlay` is true.
- `renderPhysicsDebug` now defaults to `true` in `UIState`.
- Debug color behavior: static greys (`JPH::Color::sGrey` and `sLightGrey`) are remapped to yellow in `OpenGLJoltDebugRenderer::DrawLine`.

## Session Handoff Notes (2026-05-28, Post-Process Depth + Debug Occlusion)
- Physics debug lines are drawn after `SceneViewport` render, but still depth-culled against scene geometry.
- `PostProcess::draw` now blits depth from scene FBO to default framebuffer after fullscreen post-process pass.
- `OpenGLJoltDebugRenderer::render` draws with depth test enabled and depth writes disabled (`glDepthMask(GL_FALSE)`) so overlays do not corrupt later depth consumers.

## Session Handoff Notes (2026-05-28, Collider Visualization Ownership)
- Engine no longer relies on custom `CapsuleColliderModel` rendering for capsule visualization.
- `Physics::Capsule` no longer creates/adds a debug renderable model; Jolt debug draw is the authoritative collider visualization path.
- `Capsule` sets `CharacterVirtualSettings::mInnerBodyShape = mShape` so character capsules appear in Jolt body debug draw.
- Legacy capsule draw branch was removed from `LightingPass.cpp`.

## Session Handoff Notes (2026-05-28, Default Static Colliders for Models)
- `Level::addRenderable(...)` now ensures `ModelType::ACTUAL_MODEL` instances have a default static `Physics::Box` collider and immediately syncs collider transform.
- `.model` entries loaded in `Level::loadContent(...)` now go through `addRenderable(...)` (not direct push) so default collider policy is consistently applied.
- `Model` added helper `ensureStaticBoxCollider()` and explicit sync helper `syncPhysicsColliderToModelTransform()`.

## Session Handoff Notes (2026-05-28, Physics API Simplification)
- `PhysicsObject` no longer owns a render `Model`; it now only manages physics body state.
- `PhysicsObject` now syncs from explicit transforms: `syncTransformation(position, rotation, scale)`.
- `PhysicsObject` provides body transform getters (`getWorldPosition`, `getWorldRotation`) used by `Model`.
- Removed deprecated renderable physics hooks from `Renderable`: `physicsUpdate()` and `syncTransformationToPhysicsEntity()`.
- Removed corresponding overrides/no-op implementations from `Model` and `Character`.
- `EditorWindow` no longer runs per-renderable physics hook loops; it directly updates Jolt physics each play frame.
- Removed unused `PhysicsSystemWrapper::isFirstPhysicsEnabledFrame` flag.

## Known Gotchas (Physics / Debug)
- If physics debug lines are drawn after `SceneViewport` render, they require scene depth copied to default framebuffer; keep the depth blit in `PostProcess::draw`.
- Physics debug draw is intentionally gated by `!EngineState::state->isPlay`; if debug disappears during Play, this is expected.
- Character capsules require `CharacterVirtualSettings::mInnerBodyShape = mShape` to appear in Jolt body debug draw.
- Static body debug colors may appear grey/light-grey from Jolt defaults; renderer remaps both to yellow for visibility.
- Ensure level-loaded `.model` entries go through `Level::addRenderable(...)` so default static collider attachment is applied.

