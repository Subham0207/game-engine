# FlowScript AST Unification Plan

## Document-Wide Rules

- At first mention, every new term or class name must include a one-line plain-language meaning.
- Decisions already recorded in this plan are fixed unless the user explicitly approves a change.

## Goal

Unify FlowScript compile and decompile around one common AST so both directions share the same semantics, reduce duplicated logic, and support additional target languages more easily.

Current mismatch:

- Compile path is: Visual Script (`nodes`, `links`) -> GraphIndex -> IR statements -> `LuaEmitter` -> Lua text.
- Decompile path is: Lua text -> line parser (`LuaSubsetLineParser`) -> graph builder (`DecompileGraphBuilder`) -> Visual Script.

Target path:

- Compile: Visual Script -> AST -> language renderer (start by evolving `LuaEmitter` into this role, then rename it to `LuaRenderer`).
- Decompile: Source text -> parse tree -> AST -> Visual Script.

## Why This Change

1. One intermediate model for both directions.
2. Less repeated logic between compile/decompile.
3. Better testability (roundtrip invariants on AST).
4. Easier to support future languages without rewriting graph logic.

## Scope

In scope:

- Lua-only language support in this implementation plan.
- Shared AST model used by both compile and decompile.
- Lua renderer driven by AST + templates.
- Parse-tree based decompile path (instead of current line-based subset parser).
- Lua metadata comments for node position restoration.

Out of scope (initial phases):

- Full Lua language support in one step.
- Supporting non-Lua languages.

## Existing Files to Evolve

- `Glitter/Sources/FlowScript.cpp`
- `Glitter/Sources/Compiler.cpp`
- `Glitter/Sources/LuaEmitter.cpp`
- `Glitter/Headers/NodeGraph/FlowScript/LuaEmitter.hpp`
- `Glitter/Sources/FlowScript/Decompile/LuaSubsetLineParser.cpp`
- `Glitter/Sources/FlowScript/Decompile/DecompileGraphBuilder.cpp`
- `Glitter/Headers/NodeGraph/FlowScript/Compile/IntermediateRepresentation/Expression.hpp`
- `Glitter/Headers/NodeGraph/FlowScript/Compile/IntermediateRepresentation/Statement.hpp`
- `Tests/NodeGraph/FlowScript/Compile/compiler.cpp`

## Proposed Architecture

### 1) Shared AST Layer

Introduce a common AST package for both directions:

- Expressions (initial): `NumberExpr`, `BoolExpr`, `VariableExpr`, `BinaryExpr`.
- Statements (initial): `LocalTableStmt` (table declaration), `LocalAssignStmt`, `AssignStmt` (non-local assignment, including object/field writes), `ReturnStmt`, `PrintStmt`, `FunctionStmt`.
- Metadata: optional node id, optional position, optional source span.

Notes:

- Keep compile IR classes working initially through adapter structs.
- Move gradually from compile-only IR naming toward neutral AST naming.

Required additions to close current IR/AST gaps (Phase 1):

- Add explicit field-assignment representation (do not encode field writes by overloading `LocalAssignStmt.variableName` with dotted names).
- Add AST metadata contract (`nodeId`, `visualPosition`) for roundtrip layout restore.
- Add `sourceSpan` (line/column range) for parser-produced AST nodes and diagnostics.

### 2) Compile Flow (Graph -> AST -> Lua)

- Refactor `Compiler` to build AST directly by evolving the current IR classes into shared AST classes.
- Replace emitter coupling to compile-only IR with AST input.
- Replace current emitter with template-driven `LuaRenderer` (statement templates + expression templates).
- Rename `LuaEmitter` to `LuaRenderer` and update compile entrypoint wiring in `FlowScript.cpp`.
- Render Lua using templates for syntax forms (examples):
  - `local {{ name }} = {{ value }}`
  - `{{ name }} = {{ value }}`
  - `return {{ value }}`
  - `{{ left }} {{ operator }} {{ right }}`
  - `{{ function_name }}({{ arguments }})`

### 3) Decompile Flow (Lua -> ParseTree -> AST -> Graph)

- Replace line parser as primary path.
- Parse Lua to parse tree using selected parser backend.
- Build AST from parse tree.
- Build graph from AST (reuse one graph-construction layer, not syntax-specific logic).

## Node Position Metadata in Lua

### Requirement

Persist visual script node positions in emitted Lua so decompile can restore layout.

### Initial format (comment tags)

Use stable comments that are ignored by Lua runtime, for example:

```lua
-- @fs:node id=101 x=120.0 y=260.0
local node_101 = 42
```

Alternate block-level option (if needed later):

```lua
-- @fs:layout begin
-- @fs:node id=101 x=120.0 y=260.0
-- @fs:node id=202 x=340.0 y=260.0
-- @fs:layout end
```

Rules:

- Best-effort parsing in decompile: malformed metadata should not abort code parse.
- If metadata missing, fallback to current auto-layout (`nextPos()`).

## Parser Library Exploration (C++)

The goal is parse tree generation in C++ without Java runtime dependency.

### Candidates

1. `tree-sitter-lua`
   - Pros: mature incremental parser, concrete syntax tree, robust for real-world code.
   - Cons: integration overhead, runtime dependency management.

2. PEG-based parser in C++ (`cpp-peglib` or `taocpp/PEGTL`)
   - Pros: no Java tooling, easy embedding, fast iteration for supported grammar subset.
   - Cons: grammar maintenance burden as Lua support expands.

3. `Boost.Spirit X3`
   - Pros: no external runtime dependency beyond Boost.
   - Cons: steeper grammar complexity and debug cost.

Recommendation for rollout:

- Selected backend: `tree-sitter-lua`.
- Start integration in Phase 1 with a narrow supported Lua subset, then expand coverage incrementally.

## Migration Plan (Phased)

### Phase 0 - Baseline and Contracts

- Document supported Lua subset and graph features.
- Freeze current behavior with tests.

Deliverables:

- Behavior matrix doc.
- Tests for current compile/decompile subset.

### Phase 1 - AST Introduction with Adapters

- Evolve current compile IR classes into shared AST classes.
- Keep `FlowScript::compile()` behavior unchanged externally.

Deliverables:

- AST types.
- Evolved IR-to-AST class mapping (no parallel duplicate model).
- `LuaEmitter` accepts AST (or adapter wrapper, before rename migration).
- Explicit mapping rule for field writes: IR dotted local assignment -> AST `AssignStmt`.

### Phase 2 - Graph -> AST Compiler

- Move `Compiler` internals to emit AST directly.
- Preserve diagnostics and existing node/link semantics.

Deliverables:

- `Compiler` AST output path.
- Tests for expression/exec-chain equivalence.

### Phase 3 - Metadata Emission and Restore

- Emit node position comments in Lua output.
- Parse metadata and apply spawn/grid position during decompile.

Deliverables:

- Metadata format implementation.
- Roundtrip tests verifying position persistence.

### Phase 3.5 - Renderer Migration (`LuaEmitter` -> `LuaRenderer`)

- Implement template-driven `LuaRenderer` for supported expressions/statements.
- Rename class/files/usages from `LuaEmitter` to `LuaRenderer`.
- Keep a short-lived compatibility shim only during transition.

Deliverables:

- `LuaRenderer` implementation with template mapping for current AST set.
- Compile flow switched to `LuaRenderer`.
- Legacy emitter/shim removal plan.

### Phase 4 - ParseTree-Based Decompile

- Introduce parser interface and first backend.
- Build AST from parse tree.
- Replace line parser as primary path.

Deliverables:

- `ILuaParser` abstraction.
- Parse tree -> AST mapper.
- AST -> Graph builder.

### Phase 5 - Cleanup

- Remove duplicated old paths after parity is proven.
- Keep fallback flag during stabilization window.
- Remove `LuaEmitter` compatibility shim and dotted-name field-assignment fallback.

Deliverables:

- Legacy parser deprecation/removal plan.
- Updated docs.

## Testing Strategy

Add/extend tests to cover:

1. Graph -> AST -> Lua snapshots.
2. Lua -> ParseTree -> AST validation for supported subset.
3. AST -> Graph node/link reconstruction.
4. Roundtrip invariants:
   - Graph -> Lua -> Graph structural equivalence.
   - Node position metadata retained.
5. Failure diagnostics for unsupported syntax and malformed metadata.

## Risks and Mitigations

1. Risk: parser integration delays.
   - Mitigation: parser interface + fallback backend; incremental grammar support.

2. Risk: behavior drift in compile semantics.
   - Mitigation: lock behavior with snapshot tests before migration.

3. Risk: metadata conflicts with user-authored comments.
   - Mitigation: reserved `@fs:` prefix and tolerant parser.

## Milestones

- M1: AST types + compile adapter in place.
- M2: Compiler emits AST natively.
- M3: Position metadata emitted and restored.
- M4: ParseTree-based decompile default path.
- M5: Legacy line parser retired (or kept behind feature flag temporarily).

## Immediate Next Tasks

1. Add AST namespace and core node definitions.
2. Define parser abstraction (`ILuaParser`) and implement Phase 1 backend with `tree-sitter-lua`.
3. Add metadata comment schema parser/emitter tests.
4. Add roundtrip fixture tests for a representative graph (function, print, return, binary expressions, generic fields).
5. Implement template-driven rendering in `LuaEmitter` first, then rename it to `LuaRenderer` in the migration phase.
