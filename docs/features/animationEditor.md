# Feature: Animation Editor Tool
Standalone animation-preview and timeline-region authoring tool for character prefabs. It runs as its own executable (`AnimationEditorTool`), lets users select a character + animation, preview playback (play/pause/stop/scrub), edit named timeline regions, and persist regions to animation `.meta.json` files (not binary `.animation` payloads).

## Architecture & Implementation
### Entry points and runtime bootstrap
- **`Glitter/animationEditorMain.cpp`**  
  Creates `EditorAnimation` and enters tool runtime.
- **`Glitter/Headers/EditorAnimation.hpp`**
- **`Glitter/Sources/EditorAnimation.cpp`**  
  Initializes `EngineState`, project registry/lua setup, creates `AnimationEditorWindow`, runs polling/tick loop, and handles shutdown.

### Window/tool implementation
- **`Glitter/Headers/Windowing/AnimationEditorWindow.hpp`**
- **`Glitter/Sources/Windowing/AnimationEditorWindow.cpp`**  
  Implements a `GameWindow`-derived editor window with:
  - isolated ImGui/ImNodes/GLFW context lifecycle (`init`/`tickImpl`/`shutdown`)
  - non-physics setup (`initWindowAndBackends(..., initPhysics=false)`)
  - preview scene resources: `Level`, `SceneViewport`, `Lights`, `Skybox`
  - render-only ground reference plane (`cube.fbx`, moved to **Y = -10**)
  - selection state for character/animation via engine registry maps
  - playback state (`mIsPlaying`, `mLoopAnimation`, `mScrubTimeSeconds`)
  - region editing state (`mSelectedRegionIndex`)

### Animation data model + persistence
- **`Glitter/Headers/3DModel/Animation/Animation.hpp`**
  - Added `AnimationRegion { name, startTime, endTime }`
  - Added runtime `std::vector<AnimationRegion> regions`
  - Added APIs:
    - `loadRegionsFromMeta()`
    - `saveRegionsToMeta()`
  - **Intentionally excluded regions from boost binary serialization**.
- **`Glitter/Sources/Animation.cpp`**
  - `Animation::loadAnimation(guid)` now:
    1. resolves guid via engine registry
    2. loads binary `.animation` through existing path
    3. overlays `regions` from `<guid>.meta.json`
  - Added robust region parse/write behavior:
    - missing `regions` => no-op
    - malformed region entries => skipped with logging
    - writes preserve existing metadata keys (`guid`, `type`, `version`, `content.relative_path`, custom keys)
  - Fixed debug iterator crash by using one local map instance for `find/end` comparison.

### Playback/scrubbing controls
- **`Glitter/Headers/3DModel/Animation/Animator.hpp`**
  - Added editor control helpers:
    - `SetCurrentTimeSeconds(float)`
    - `GetCurrentTimeSeconds() const`
    - `SetPlaying(bool)`
  - Enables deterministic timeline scrub and explicit play/pause control from UI.

### Character spawning for tool-mode preview
- **`Glitter/Headers/Level/Level.hpp`**
- **`Glitter/Sources/Level.cpp`**
  - Extended `Level::spawnCharacter(...)` with `enableGameplaySystems` (default `true`).
  - Animation editor calls with `false` to avoid gameplay-side systems in preview mode.
  - When `false`, player controller/camera/capsule setup is bypassed for lightweight preview.

### Build/test integration
- **`CMakeLists.txt`**
  - Added tool target:
    - `add_executable(AnimationEditorTool ${CMAKE_SOURCE_DIR}/Glitter/animationEditorMain.cpp)`
    - `target_link_libraries(AnimationEditorTool PRIVATE GlitterLib)`
  - Added install packaging for `AnimationEditorTool`.
- **`Tests/Animation/Animation.test.cpp`**
  - Added coverage for:
    - loading regions from `.meta.json` (with invalid-entry skipping)
    - saving regions while preserving existing meta fields
- **`Tests/CMakeLists.txt`**
  - Registered new animation region tests in `Test` target.

### Data flow
1. `AnimationEditorTool` boots `EngineState` + registry + Lua, then opens `AnimationEditorWindow`.
2. Window queries:
   - characters from `EngineState::state->engineRegistry->characterPrefabMap`
   - animations from `EngineState::state->engineRegistry->animationsFileMap`
3. Character selection spawns preview character in tool `Level` (`enableGameplaySystems=false`).
4. Animation selection calls `Animation::loadAnimation(guid)`; binary content loads first, then meta regions overlay into `Animation::regions`.
5. Playback UI drives animator (`SetPlaying`, `SetLoopCurrentAnimation`, `SetCurrentTimeSeconds`); scrubbed/current time round-trips through animator.
6. Region UI mutates `Animation::regions` in memory; explicit **Save Regions** writes `regions[]` back into `<guid>.meta.json`.

## Design Decisions & "Why"
- **Standalone executable vs in-editor spawn/focus**: avoids prior multi-window same-process scaling issues and keeps context ownership simpler/safer (GLFW+ImGui isolation per tool process).
- **Meta JSON as sole region persistence**: region authoring is editor metadata, not skeletal animation payload; keeping it out of binary serialization avoids schema coupling and preserves `.animation` compatibility.
- **Non-physics preview**: animation authoring task does not need simulation; disabling physics reduces side effects/perf cost and avoids gameplay-system coupling.
- **Animator helper API**: explicit time/play controls are required for deterministic scrub UX; relying only on runtime tick progression was insufficient.
- **Preserve unknown meta keys on save**: avoids destructive writes and keeps future metadata extensibility.
- **Explicit save for regions**: chosen to keep authoring intent clear and avoid accidental churn/noisy disk writes.

## Technical Debt & Edge Cases
- **Global `EngineState::isPlay` coupling**: tool currently sets `EngineState::state->isPlay = true` to drive some existing animation paths; this is a cross-tool global side effect and should be decoupled into tool-local preview mode.
- **Region timeline interaction is basic**: bars are visualized, but no drag-resize handles on bars yet; edits are currently input-field based.
- **Region validation policy intentionally permissive**: overlaps and duplicate names are allowed by design; downstream event dispatch systems must resolve ambiguity deterministically.
- **No undo/redo for region edits**: destructive edits (including Delete key) are immediate in-memory until save.
- **Preview character replacement path**: existing preview character is removed by pointer identity from level renderables; future refactors should ensure instance-map cleanup remains consistent.
- **Meta file assumptions**: region save resolves target by asset GUID through registry mapping; stale/missing registry entries return failure (`false`) and skip write.
- **Input UX details**: region list spacing/padding required manual UI adjustments (added top row padding) to avoid header overlap artifacts.
