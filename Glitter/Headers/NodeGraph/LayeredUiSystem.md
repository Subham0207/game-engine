# Layered NodeGraph UI System (Views + Layers)

## NodeGraph folder structure (where to add things)

Current NodeGraph code lives here:

```
Glitter/
  Headers/
    NodeGraph/
      Components/
        CommentBox.hpp        // Model: comment box data (position, size, etc)
        NodeGraphNode.hpp     // Model: wrapper around an ImNodes node id + metadata
      Views/
        NodeGraphCommentsView.hpp // View: draws + edits CommentBox
        NodeGraphNodesView.hpp    // View: draws + edits ImNodes-backed nodes
      INodeGraphView.hpp      // View interface (layer/priority/draw)
      NodeGraphLayer.hpp      // Global draw ordering categories
      NodeGraphEditorSpace.hpp// grid<->screen conversion helpers + captured editor space
      NodeGraphRenderContext.hpp // per-frame snapshot + interaction ownership state
      NodeGraphViewRegistry.hpp  // registers and draws all views (sorted)
      NodeGraphContextMenu.hpp   // popup view (adds elements)
      NodeGraph.hpp           // orchestrator: owns models + registry + builds render context
      LayeredUiSystem.md      // this document
```

When creating a new **node graph component** you will typically:

- add a **model** type under `Components/`
- add a **view** under `Views/`
- register the view in `NodeGraph.hpp` (usually one line in the constructor)

## What is a “Node Graph Component” in this codebase?

In this project, a **node graph component** means:

> A *semantic element type* that lives inside the node editor and participates in its rendering and interaction.

A component typically has:

- **A model type** (stored/owned by `NodeGraph`) e.g.
  - `NodeGraphNode` (wrapper around an ImNodes node id + name + position bookkeeping)
  - `CommentBox` (custom drawn rectangle in the editor background)
- **A dedicated view** (`INodeGraphView`) that:
  - renders the component
  - owns the component’s *UI state* (selection, dragging, editing)
  - often owns the component’s *creation logic* / id allocation (e.g. `nextNodeId`, `nextCommentId`)

So, conceptually:

**Component (data) + View (render + interaction + creation)**

This separation keeps `NodeGraph` as a small orchestrator and makes it easy to add new element types without
reworking draw ordering or input conflicts.

### Existing components

- **ImNodes-backed node component:** `NodeGraphNode` rendered by `NodeGraphNodesView`
- **Custom drawn background component:** `CommentBox` rendered by `NodeGraphCommentsView`
- **Popup component:** `NodeGraphContextMenu` (not a “graph element”, but still a layered view)

---

## Creating a new Node Graph Component (recommended recipe)

Use this checklist when adding a new element type (e.g. “State”, “Group”, “Selection rectangle”, “Debug overlay”).

1. **Create/choose the model type**
   - Define a struct/class for persistent data (ids, positions, properties).
   - Store a container owned by `NodeGraph` (or a pointer/reference in `NodeGraphRenderContext`).

2. **Create a view for the component**
   - Implement `INodeGraphView`.
   - Pick the correct `NodeGraphLayer`:
     - `Background` for behind-ImNodes custom drawing
     - `Content` for ImNodes nodes
     - `Overlay` for selection, minimap, link previews, etc.
     - `Popup` for context menus
   - Use `priority()` sparingly to order within the same layer.

3. **Put UI state and creation logic in the view**
   - Keep transient state (hovered/selected/dragging/editing ids) **inside the view**.
   - Prefer to keep id generation + creation helpers in the view as well (e.g. `addXxx()`, `nextXxxId`).

4. **Register the view once in `NodeGraph::NodeGraph()`**
   ```cpp
   views.emplaceView<MyNewComponentView>(/*optional ctor args*/);
   ```

5. **(Optional) Add a context menu entry to create the component**
   - Add a callback to `NodeGraphContextMenu` (or a new popup view).
   - In the callback, use `views.findView<MyNewComponentView>()` and call the view’s `add…()` API.

6. **Use interaction ownership to avoid conflicts**
   - Before starting drag/resize, call `ctx.interaction.tryClaim(owner, priority)`.
   - While owned, other views should treat themselves as blocked.
   - Release on mouse release.

This recipe avoids the most common regressions:
- layering bugs (elements drawing above/below each other unexpectedly)
- input conflicts (multiple things dragging at once)

### Minimal example (model + view + registration)

Below is a **small** example of a custom background element (similar in spirit to `CommentBox`).
This is intentionally short and focuses only on the pattern.

1) **Model** (owned by `NodeGraph`)

```cpp
struct HeatmapCell
{
    int id = -1;
    ImVec2 posGrid{0,0};
    float intensity = 0.0f;
};
```

2) **View** (renders + owns UI state + creation)

```cpp
class HeatmapView final : public INodeGraphView
{
public:
    explicit HeatmapView(std::vector<HeatmapCell>& cells) : m_cells(cells) {}

    NodeGraphLayer layer() const override { return NodeGraphLayer::Background; }
    int priority() const override { return 0; }

    int nextId = 0;
    void addCell(const ImVec2& atGrid, float intensity)
    {
        m_cells.push_back(HeatmapCell{nextId++, atGrid, intensity});
    }

    void draw(NodeGraphRenderContext& ctx) override
    {
        if (!ctx.editorHovered)
            return;

        // Block if someone else currently owns interaction.
        if (!ctx.interaction.canInteract(NodeGraphInteractionOwner::Other))
            return;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        for (const auto& cell : m_cells)
        {
            const ImVec2 p0 = NodeGraphGridToScreen(ctx.editorSpace, cell.posGrid);
            const ImU32 col = IM_COL32(255, 0, 0, (int)(cell.intensity * 255));
            dl->AddRectFilled(p0, ImVec2(p0.x + 20, p0.y + 20), col);
        }
    }

private:
    std::vector<HeatmapCell>& m_cells;
};
```

3) **Register once** (in `NodeGraph::NodeGraph()`)

```cpp
views.emplaceView<HeatmapView>(heatmapCells);
```

4) **(Optional) Create it from the context menu**

- Add a callback + menu item in `NodeGraphContextMenu`.
- In the callback, locate the view and call `addCell(...)`.

This is the same pattern used by the built-in components.

---

This node editor UI is intentionally split into **small “views”** (nodes / comments / context menu / future overlays) and drawn through a **layered pipeline**.

The goal is:
- **Stable draw order** (no more “comments cover nodes” regressions)
- Simple extensibility: adding a new element type should be a **one-time view class + one registration line**
- Keep NodeGraph itself as a small **orchestrator**

---

## Core idea

Instead of hard-coding:

```cpp
comments.draw(...);
nodes.draw(...);
contextMenu.draw(...);
```

`NodeGraph` registers views into a registry. Each view declares which **layer** it belongs to (Background / Content / Overlay / Popup). The registry sorts by layer (+ optional priority) and draws them every frame.

---

## Important files

### 1) `NodeGraphLayer.hpp`
Defines the global ordering:
- `Background` – draws first (behind)
- `Content` – main content (nodes)
- `Overlay` – selection rectangles, minimap, debug overlays
- `Popup` – context menus, popups

Smaller enum value draws earlier.

### 2) `NodeGraphRenderContext.hpp`
A small struct passed to every view each frame:
- `editorSpace` – stable grid ↔ screen mapping captured at the start of the frame
- per-frame input snapshot (captured once by `NodeGraph` so views don’t re-query ImGui/ImNodes):
  - `editorHovered`
  - `mouseScreen`
  - `leftClicked`, `leftDoubleClicked`, `leftDown`, `leftReleased`
- references to model containers:
  - `nodes`
  - `comments`

It also contains the **interaction ownership** state:

- `interaction` (`NodeGraphInteractionState`)

This is what prevents multiple views (e.g. ImNodes nodes + comment boxes + future overlays)
from fighting over the same mouse drag.

If future elements need more shared data, add it here **only when needed**.

### 3) `INodeGraphView.hpp`
The minimal interface every view implements:

- `layer()` → where it draws
- `priority()` → optional ordering within a layer (smaller first)
- `draw(ctx)` → draw/update UI for that view

### 4) `NodeGraphViewRegistry.hpp`
Owns the views (`unique_ptr`).

- `emplaceView<T>()` registers a view
- `drawAll(ctx)` draws them in stable sorted order

---

## How the frame is drawn

Inside `NodeGraph::drawUI()`:

1. `ImGui::Begin(...)`
2. `ImNodes::BeginNodeEditor()`
3. Capture per-frame input into `renderCtx` **once**:
   - `renderCtx.mouseScreen = ImGui::GetMousePos();`
   - `renderCtx.editorHovered = ImNodes::IsEditorHovered();`
   - `renderCtx.leftClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);`
   - `renderCtx.leftDoubleClicked = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);`
   - `renderCtx.leftDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);`
   - `renderCtx.leftReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);`
4. Update interaction ownership state:
   - owner is kept **sticky while mouse is down**
   - owner is cleared on mouse release (so another view can claim next)
5. Capture editor space:
   - `editorSpace = NodeGraphEditorSpace::CaptureFromCurrentEditor();`
6. Update `renderCtx.editorSpace` for this frame
7. `views.drawAll(renderCtx);`
8. `ImNodes::EndNodeEditor()`
9. `ImGui::End()`

The view registry handles ordering:

`Background → Content → Overlay → Popup`

---

## Existing views

### `NodeGraphCommentsView`
- Implements `INodeGraphView`
- `layer() == NodeGraphLayer::Background`
- Draws comment rectangles using `ImDrawList`.

Interaction notes:
- When starting a drag/resize it tries to **claim** ownership:
  - `ctx.interaction.tryClaim(NodeGraphInteractionOwner::Comments, priority)`
- While dragging it assumes it is the owner.
- On mouse release it **releases** ownership:
  - `ctx.interaction.release(NodeGraphInteractionOwner::Comments)`

### `NodeGraphNodesView`
- Implements `INodeGraphView`
- `layer() == NodeGraphLayer::Content`
- Submits ImNodes nodes.

Interaction notes:
- If ImNodes is hovered/active it claims ownership as `ImNodes`.
  This keeps ImNodes-specific queries localized to the nodes view.

### `NodeGraphContextMenu`
- Implements `INodeGraphView`
- `layer() == NodeGraphLayer::Popup`
- Uses callbacks set by `NodeGraph` to add nodes/comments.

Interaction notes:
- The popup/menu should generally take ownership while open (or at least block other
  interactions) so clicks don’t leak to other views.

Why callbacks?
- The menu is a view and should not directly depend on *specific* component views.
- Callbacks keep the menu reusable: it can request creation without needing to know where the creation logic lives.
- In the current design, **each component view** typically owns its own id allocation + `add...()` helpers.

---

## What this document removed / de-duplicated

- Removed duplicate re-statements of the same “views + layers” concept that appeared multiple times in the middle of the file.
- Kept a single **minimal example** (model + view + registration) since that is still useful as a template.

## Notes / design rules

- **Do not** reorder model vectors to “bring to front” for layering.
  - Layering should come from the registry and view layers.
- If you need ordering inside a layer, use `priority()`.
- Keep `NodeGraphRenderContext` small;
  add new fields only when a new view truly needs shared state.

### Interaction ownership rules (important)

**Problem:** multiple views exist in the same editor (ImNodes nodes, comments, context menus,
future selection tools). Without coordination, they all see the same mouse press and may all
start dragging / selecting simultaneously.

**Solution:** `NodeGraphInteractionState` provides an exclusive “who owns the mouse right now?”
mechanism.

Key types (from `NodeGraphRenderContext.hpp`):

- `NodeGraphInteractionOwner` (examples: `None`, `ImNodes`, `Comments`, `ContextMenu`, `Other`)
- `NodeGraphInteractionState`:
  - `owner`, `ownerPriority`
  - `tryClaim(owner, priority)`
  - `canInteract(owner)` / `isOwnedBy(owner)` / `isOwnedByOtherThan(owner)`
  - `release(owner)`

**Recommended pattern inside a view:**

1. Early-out (or ignore input) if you are blocked:

```cpp
if (!ctx.interaction.canInteract(NodeGraphInteractionOwner::Comments))
    return;
```

2. When you detect a “begin interaction” gesture (mouse down on your element), claim ownership:

```cpp
if (ctx.leftClicked)
    ctx.interaction.tryClaim(NodeGraphInteractionOwner::Comments, 20);
```

3. On mouse release, release ownership (or rely on the orchestrator clearing it):

```cpp
if (ctx.leftReleased)
    ctx.interaction.release(NodeGraphInteractionOwner::Comments);
```

**Priority convention:**
- Higher priority wins if multiple views attempt to claim in the same frame.
- Keep priorities small and consistent (e.g. ImNodes ~10, comments ~20, selection tools ~30).

