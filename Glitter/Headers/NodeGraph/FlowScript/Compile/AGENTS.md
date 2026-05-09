# AGENTS - FlowScript Compile

This file tracks the current `Visual Script -> AST -> Lua` workstream status and the next implementation steps.

## Scope
- Applies to `Glitter/Headers/NodeGraph/FlowScript/Compile/*` and related compile sources.
- Focus is FlowScript compile pipeline behavior and AST/transpiler conventions.

## What Was Implemented

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

### Visual Graph -> AST Wiring
- Updated AST builder to read:
  - function name from visual function node
  - variable declaration metadata (`name`, `declaredType`, `value`)
  - get-variable metadata (`variableName`)
  - node positions (`spawnPosScreen`) into AST `x,y`
- Added AST mapping for new expression node types (`Multiply`, `Divide`, `Modulo`, `LessThan`).

### Compatibility Updates (Current Compile/Decompile Path)
- Extended legacy compiler support for `Multiply`, `Divide`, `Modulo`, `LessThan`.
- Extended Lua subset decompiler binary-expression parser for same operators.

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
- Node metadata editing UI is still minimal (defaults are present, richer property panels pending).
- Serialized position string is generated and parseable, but full save/load round-trip binding to persisted script artifacts is still pending.
- No field/dot-path node yet (`a.b.c`) for table member access.

## Pending Next Steps (Priority Order)

### 1) Editor Metadata Panels
- Add property UI for:
  - function name
  - variable declaration name/type/value
  - get-variable name
- Add inline validation feedback for required fields.

### 2) Position Round-Trip Integration
- Connect `serializedNodePositions` to persisted script storage.
- On load/decompile, parse and apply positions deterministically to created nodes.
- Add regression tests for save/load position round-trip.

### 3) Reverse Flow: `Lua -> AST -> Visual Script`
- Build parser/decompiler path that reconstructs typed AST first.
- Materialize graph nodes/links from typed AST.
- Keep coverage aligned with currently emitted Lua subset.

## Suggested Milestone Plan
1. Editor metadata UI + validation.
2. Position persistence round-trip integration.
3. Round-trip subset (`Visual -> AST -> Lua -> AST -> Visual`) with fixtures.
4. Expand language/node coverage incrementally (tables/records/arrays/field access).

## Guardrails
- Keep function name required in AST and transpiler.
- Keep declaration variable name required.
- Keep typed AST nodes as source of truth (avoid reintroducing generic stringly-typed payloads).
- Maintain execution-chain stop at return in emitted Lua.
