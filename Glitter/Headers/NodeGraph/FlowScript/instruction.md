# FlowScript Notes

This file documents the recent FlowScript changes and how the compile step works.

## Recent changes

- Added an Execute button to the FlowScript UI and wired it to run the compiled Lua chunk.
- Added a small Lua log panel in the FlowScript UI. Lua `print(...)` output is captured and shown here.
- Added basic keyword nodes and compile support for:
  - `Function` (starts a Lua function body).
  - `Print` (emits `print("[LUA]", value)` and can sit in the exec flow).
  - `Return` (emits `return <expr>` and ends the exec flow chain).
- Added execution flow pins to relevant nodes and updated compilation to respect exec flow order.
- Disabled auto-return when a `Return` node is present (explicit return only).
- Added `FlowScript::deCompile` to rebuild a node graph from supported Lua.

## How compilation works (FlowScript::compile)

Below is a high-level, start-to-finish explanation of the compile pipeline.

1) Collect nodes and attributes
- Gather all nodes into a flat list.
- Build a lookup of every attribute id -> {node, input/output, type}.
- Execution pins (ExecutionFlowInPin / ExecutionFlowOutPin) are tracked separately.

2) Split links into two graphs
- For each link:
  - If it connects ExecutionFlowOutPin -> ExecutionFlowInPin, store it in the exec-flow map.
  - If it connects data pins (PIN/FIELD -> PIN), store it in the data-flow map.

3) Build data dependencies
- For each node input PIN, follow its data link to the source output.
- Build a dependency graph so data-producing nodes appear before data-consuming nodes.
- Run a topological sort to get a safe ordering for data nodes.

4) Assign Lua variable names
- Every node gets a unique local name: `node_<id>`.
- This allows data nodes to reference each other by variable name.

5) Detect explicit returns
- If any `Return` node exists, auto-return is disabled.

6) Emit data-only nodes
- Nodes without exec pins are emitted first in the topo order.
- Example: `Integer` becomes `local node_12 = 5`.

7) Build execution flow chains
- Build an exec-flow adjacency list from exec links.
- Walk from exec-flow roots (nodes with exec pins and no exec input link).
- Emit statements in that order:
  - `Function` opens `local node_x = function()` and increases indent.
  - `Print` emits `print("[LUA]", expr)`.
  - `Return` emits `return expr` and stops the chain.
- After a `Function` chain finishes, emit `end` to close the function body.

8) Emit auto-return when allowed
- If there is no explicit `Return`, the compiler returns terminal data nodes:
  - One terminal value: `return node_x`
  - Many: `return { node_a, node_b, ... }`

9) Store result
- The final Lua source is stored in `compiledLua` and returned.

## How de-compilation works (FlowScript::deCompile)

`deCompile` parses a limited Lua subset (the same patterns emitted by `compile`) and rebuilds nodes/links.
If it encounters unsupported Lua, it throws a runtime error.

Supported statements and mappings:
- `local node_x = <number>` -> `Integer` node with field set.
- `local node_x = <true|false>` -> `Boolean` node with field set.
- `local node_x = (<a> + <b>)` -> `Add` node with inputs linked.
- `local node_x = (<a> - <b>)` -> `Subtract` node with inputs linked.
- `local node_x = (<a> > <b>)` -> `GreaterThan` node with inputs linked.
- `local node_x = (<a> == <b>)` -> `EqualsTo` node with inputs linked.
- `local node_x = (<a> ~= <b>)` -> `NotEqualsTo` node with inputs linked.
- `print("[LUA]", <expr>)` -> `Print` node in exec flow.
- `return <expr>` -> `Return` node in exec flow (single input only).
- `local node_x = function()` ... `end` -> `Function` node and exec chain.

Execution flow reconstruction:
- `Function` blocks create a `Function` node and link its exec output to the first
  exec statement inside the block.
- Inside a function, `Print` and `Return` are appended to the exec chain in
  source order and linked through exec pins.
- At top level (outside functions), exec statements are linked in source order.

Data value resolution:
- `<expr>` can be a literal number/boolean or a previously defined `node_<id>` variable.
- Unsupported literals (strings, tables, etc.) cause a throw.

Limitations:
- `return { ... }` is not supported.
- No attempt is made to parse arbitrary Lua; only the supported subset is accepted.

## Notes

- `Print` is part of the execution flow, so its exec output can chain to the next node.
- The FlowScript UI console shows `print(...)` output using `setPrintHandler`.
- If you want a single, global log, move the handler setup out of the Execute button.
