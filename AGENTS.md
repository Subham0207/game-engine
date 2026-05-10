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
