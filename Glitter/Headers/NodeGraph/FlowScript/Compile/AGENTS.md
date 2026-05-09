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

### Visual Graph -> AST Wiring
- Extended graph node enum (`NodeTypes`) with:
  - `VariableDeclaration`
  - `GetVariable`
- Added visual graph node classes:
  - `NodeGraphNodes/Variables/VariableDeclaration.hpp`
  - `NodeGraphNodes/Variables/GetVariable.hpp`
- Updated `NodeGraphNodeFactory` to build new node types.
- Added explicit function identifier on visual function node (`Function::functionName`).
- Updated AST builder to read:
  - function name from visual function node
  - variable declaration metadata (`name`, `declaredType`, `value`)
  - get-variable metadata (`variableName`)

### Tests
- Updated compile/transpiler tests for strict function-name behavior.
- Added tests for:
  - return chain termination
  - function without params
  - variable declaration/get-variable transpilation
  - additional binary operators
  - graph-to-AST for variable declaration/get-variable

## Current Constraints / Known Gaps
- Some static-analysis warnings remain (non-fatal); build/tests pass.
- New visual node classes currently carry metadata fields but are not yet fully integrated into editor UI interaction and serialization formats.
- No field/dot-path node yet (`a.b.c`) for table member access.

## Pending Next Steps (Priority Order)

### 1) Extend Visual Script Editor Support (Recommended Immediate Next Step)
- Add node creation menu entries for:
  - variable declaration
  - get variable
  - less-than, multiply, divide, modulo (if missing in editor surface)
- Add editor-side property panels for new metadata:
  - function name
  - variable declaration name/type/value
  - get variable name
- Ensure pin/link constraints match intended data flow (exec vs data pins).

### 2) Serialization + De-serialization of Graph UI Positions
- Persist each node's position to saved representation.
- Restore exact position on load so reopened scripts preserve layout.
- Include regression test coverage for round-trip position persistence.

### 3) Reverse Flow: `Lua -> AST -> Visual Script`
- Build parser/decompiler path for Lua subset currently emitted by transpiler.
- Reconstruct typed AST first, then materialize graph nodes/links.
- Start with same subset already supported by transpiler to guarantee round-trip viability.

## Suggested Milestone Plan
1. Editor support + metadata wiring for new nodes.
2. Position persistence during save/load.
3. Round-trip subset (`Visual -> AST -> Lua -> AST -> Visual`) with fixture tests.
4. Expand language/node coverage incrementally (tables/records/arrays/field access).

## Guardrails
- Keep function name required in AST and transpiler.
- Keep declaration variable name required.
- Keep typed AST nodes as source of truth (avoid reintroducing generic stringly-typed payloads).
- Maintain execution-chain stop at return in emitted Lua.

