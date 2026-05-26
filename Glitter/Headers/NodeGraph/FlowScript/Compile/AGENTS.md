# AGENTS - FlowScript Compile

This file tracks the current `Visual Script -> AST -> Lua` workstream status and the next implementation steps.

## Scope
- Applies to `Glitter/Headers/NodeGraph/FlowScript/Compile/*` and related compile sources.
- Focus is FlowScript compile pipeline behavior and AST/transpiler conventions.

## What Was Implemented

### Recent Updates (2026-05-25)
- Added `FlowScript::compile()` regression coverage for:
  - generated banner prefix behavior
  - compile error capture into diagnostics
  - diagnostics clear-on-recompile behavior
- Added `LuaTranspiler::Transpile` regression coverage for:
  - `GreaterThan` and `NotEqualsTo` expression emission inside `return`
  - deserialized `.flowscript` graphs (including function/return wiring)
- Fixed AST exec traversal behavior:
  - statement expression inputs are resolved before exec traversal stop checks
  - exec passthrough nodes continue traversal to the next exec-linked node
- Fixed FlowScript deserialize value restoration:
  - serialized FIELD values are reapplied by attribute id during graph load
- Fixed loaded-node position persistence:
  - deserialized nodes keep `positionSet=false` so first draw applies serialized screen positions via ImNodes

### Cross-Module Update (2026-05-26)
- NodeGraph editor now deletes selected nodes on `Delete` (regular graph nodes + state-machine nodes), and removes attached links/transitions in the same operation.
- Hotkey handling skips delete while text input is active (`ImGui::GetIO().WantTextInput`) to avoid accidental graph mutation during metadata editing.
- Regression tests were added in `Tests/NodeGraph/NodeGraph.test.cpp` for both delete paths.

### AST Model Refactor
- Replaced generic AST payload style with typed AST node hierarchy (`AstNode` base).
- Statement branch includes:
  - `FunctionStatementAstNode`
  - `PrintStatementAstNode`
  - `ReturnStatementAstNode`
  - `VariableDeclarationStatementAstNode`
- Expression branch includes:
  - `IntegerLiteralExpressionAstNode`
  - `BooleanLiteralExpressionAstNode`
  - `GetVariableExpressionAstNode`
  - `Add/Subtract/Multiply/Divide/Modulo`
  - `EqualsTo/NotEqualsTo/GreaterThan/LessThan`
- Added `x` / `y` coordinates on `AstNode` base so all AST nodes carry editor position metadata.

### Lua Transpiler Behavior Changes
- `print` no longer defaults to `'Hello world'`; empty input emits `print()`.
- `return` terminates chain emission (no trailing statement emission after return node).
- Function name is mandatory during transpilation (`throw` if missing).
- Function parameters no longer default to `x`; empty list emits `function name()`.
- Added expression emit support for `<`, `*`, `/`, `%`, and `GetVariable`.
- Added variable declaration emission support:
  - `local <name> = <value>`
  - Throws when declaration name is missing.
  - Type-based default value emission when declaration value is empty.
- `LuaTranspiler::Transpile` now returns `LuaTranspileOutput`:
  - `luaCode`
  - `serializedNodePositions`

### Visual Script Surface Extension
- Extended graph node enum (`NodeTypes`) with:
  - `Multiply`, `Divide`, `Modulo`, `LessThan`
  - `VariableDeclaration`, `GetVariable`
- Added visual graph node classes:
  - `NodeGraphNodes/BinaryOperators/Multiply.hpp`
  - `NodeGraphNodes/BinaryOperators/Divide.hpp`
  - `NodeGraphNodes/BinaryOperators/Modulo.hpp`
  - `NodeGraphNodes/BinaryOperators/LessThan.hpp`
  - `NodeGraphNodes/Variables/VariableDeclaration.hpp`
  - `NodeGraphNodes/Variables/GetVariable.hpp`
- Updated context menu + node factory to create all above nodes.
- Added defaults for quick usability:
  - `Function::functionName = "foo"`
  - Variable declaration/get-variable default names/types.

### Position Serialization Utility
- Added `NodePositionSerialization` utility:
  - `Serialize(std::vector<NodePosition>) -> string`
  - `Deserialize(string) -> std::vector<NodePosition>`
- Utility is used by `LuaTranspiler` when building `serializedNodePositions`.
- `NodeGraphNodesView` now writes live node screen position back to node model every frame.
- Graph JSON load path relies on serialized node `x/y` in `.flowscript`; first-draw position application is required for positions to persist visually.

### Editor Metadata UI
- Added inline metadata editing in `NodeGraphNodesView` for:
  - function node `functionName`
  - variable declaration `variableName`, `declaredType`, `value`
  - get-variable `variableName`
- Added required-field warning text in-node when required metadata is empty.
- Updated node output-field ImGui IDs to include node id to avoid ID collisions.

### Visual Graph -> AST Wiring
- Updated AST builder to read:
  - function name from visual function node
  - variable declaration metadata (`name`, `declaredType`, `value`)
  - get-variable metadata (`variableName`)
  - node positions (`spawnPosScreen`) into AST `x,y`
- Added AST mapping for new expression node types (`Multiply`, `Divide`, `Modulo`, `LessThan`).

### One-Way Pipeline Migration
- `FlowScript::compile()` now uses typed AST + `LuaTranspiler` directly (legacy `Compiler` + `LuaEmitter` path removed from FlowScript compile entrypoint).
- FlowScript Lua decompile API was removed.
- State-machine transition editing now restores graphs from `.flowscript` via `VisualScriptJsonSerializer::DeserializeFromFile`.
- Lua decompile source/header modules under `FlowScript/Decompile` were deleted from the repository.
- Added `Compile All` action in state-machine toolbar to compile every transition FlowScript to Lua artifact.

### Build System Fixes
- Fixed `ALL_BUILD` linker failure (`LNK2019`/`LNK1120`) caused by missing link of `NodePositionSerialization` implementation.
- Added `Glitter/Sources/NodePositionSerialization.cpp` to `GlitterLib` sources in root `CMakeLists.txt`.

### Tests
- Updated transpiler tests for `LuaTranspileOutput` contract.
- Added/updated tests for:
  - return chain termination
  - function without params
  - variable declaration/get-variable transpilation
  - additional binary operators
  - graph-to-AST for variable declaration/get-variable
  - AST node position capture
  - node-position utility serialize/deserialize/validation

## Current Constraints / Known Gaps
- Some static-analysis warnings remain (non-fatal); build/tests pass.
- `LuaTranspileOutput.serializedNodePositions` still exists but editor source-of-truth is `.flowscript` JSON node positions.
- No field/dot-path node yet (`a.b.c`) for table member access.
- Runtime transition Lua is wrapper-based (`return function(t) ... end`) around generated FlowScript body.
- Continue adding integration tests that deserialize real graph JSON payloads to prevent regressions between serializer, AST builder, and transpiler.

## Pending Next Steps (Priority Order)

### 1) Compile-All UX + Diagnostics
- Surface per-link success/failure summary in UI after batch compile.
- Expose failed link IDs/paths for quick jump and fix.

### 2) Deterministic Artifact Output
- Ensure stable ordering/newline behavior for generated Lua across repeated compiles.
- Add regression tests that compile the same `.flowscript` twice and assert identical `.lua` output.

### 3) FlowScript Serialization Hardening
- Add malformed JSON + missing required field tests for `VisualScriptJsonSerializer`.
- Validate link/node id references during deserialize and report actionable errors.

## Suggested Milestone Plan
1. Batch-compile diagnostics and editor UX polish.
2. Deterministic Lua artifact test coverage.
3. Serializer/deserializer validation hardening.
4. Expand language/node coverage incrementally (tables/records/arrays/field access).

## Guardrails
- Keep function name required in AST and transpiler.
- Keep declaration variable name required.
- Keep typed AST nodes as source of truth (avoid reintroducing generic stringly-typed payloads).
- Maintain execution-chain stop at return in emitted Lua.
