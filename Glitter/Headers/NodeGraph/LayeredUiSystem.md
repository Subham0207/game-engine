# Layered NodeGraph UI System (Views + Layers)

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
- references to model containers:
  - `nodes`
  - `comments`

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
3. Capture editor space:
   - `editorSpace = NodeGraphEditorSpace::CaptureFromCurrentEditor();`
4. Update `renderCtx.editorSpace` for this frame
5. `views.drawAll(renderCtx);`
6. `ImNodes::EndNodeEditor()`
7. `ImGui::End()`

The view registry handles ordering:

`Background → Content → Overlay → Popup`

---

## Existing views

### `NodeGraphCommentsView`
- Implements `INodeGraphView`
- `layer() == NodeGraphLayer::Background`
- Draws comment rectangles using `ImDrawList`.

### `NodeGraphNodesView`
- Implements `INodeGraphView`
- `layer() == NodeGraphLayer::Content`
- Submits ImNodes nodes.

### `NodeGraphContextMenu`
- Implements `INodeGraphView`
- `layer() == NodeGraphLayer::Popup`
- Uses callbacks set by `NodeGraph` to add nodes/comments.

Why callbacks?
- The menu is a view, but `NodeGraph` owns the model and ID allocation (`nextNodeId`, etc.).

---

## How to add a new element type

Example: add a minimap overlay.

### Step 1: create a new view
Create `NodeGraphMiniMapView.hpp`:

```cpp
class NodeGraphMiniMapView final : public INodeGraphView
{
public:
    NodeGraphLayer layer() const override { return NodeGraphLayer::Overlay; }

    // Optional: draw above other overlays
    // int priority() const override { return 10; }

    void draw(NodeGraphRenderContext& ctx) override
    {
        // Use ctx.editorSpace + ctx.nodes/comments to render a minimap.
        // Keep input handling localized to this view.
    }
};
```

### Step 2: register it once
In `NodeGraph::NodeGraph()`:

```cpp
views.emplaceView<NodeGraphMiniMapView>();
```

That’s it. The registry will draw it automatically every frame in the correct layer.

---

## Notes / design rules

- **Do not** reorder model vectors to “bring to front” for layering.
  - Layering should come from the registry and view layers.
- If you need ordering inside a layer, use `priority()`.
- Keep `NodeGraphRenderContext` small;
  add new fields only when a new view truly needs shared state.

