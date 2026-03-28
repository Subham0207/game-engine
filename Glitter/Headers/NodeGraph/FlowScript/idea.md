What you need from here

You need 5 layers:

Graph model
Your existing nodes, pins, links
Graph query helpers
Functions to answer questions like:
what is connected to this pin?
what node owns this pin?
what node executes next?
Validation pass
Check if graph is legal before codegen
Code generation model
A small internal representation of expressions/statements
Lua emitter
Convert that representation into Lua text
The biggest decision: expression flow vs execution flow

Your graph already has two different systems:

execute pins → statement order
attribute pins → data flow

That is correct.

So you should map them like this:

Execution pins represent statements

Examples:

print
return
function body
if
while
assignment

These are things that happen in order.

Attribute pins represent expressions / values

Examples:

int literal
bool literal
add
subtract
comparison
variable reference

These produce values.

That distinction is the key. If you keep it clean, everything becomes easier.

First thing to do: classify node categories better

Your current folders are okay for organization, but for code generation you need semantic categories.

Instead of only:

keyword
datatype
binary operator

also give each node a codegen kind.

For example:

enum class NodeKind
{
LiteralInt,
LiteralBool,
Print,
Return,
Add,
Subtract,
Multiply,
Divide,
Function,
Parameter,
VariableGet,
VariableSet,
If,
While,
CompareEqual,
CompareLess,
};

Your concrete classes can still exist, but every node should expose a semantic kind.

Example:

class Node
{
public:
virtual NodeKind GetKind() const = 0;
};

This matters because codegen should not care that a node lives in a “keyword folder.” It should care that it is a Print node or an Add node.

Second: define pin roles clearly

Right now you have input/output pins and execute pins. Good. But codegen becomes much easier if each pin has a semantic meaning, not just direction.

Example:

enum class PinDirection
{
Input,
Output
};

enum class PinValueKind
{
Flow,       // execute pins
Data,       // connected data
Field       // inline constant typed by user
};

And ideally each pin should also have a name:

struct Attribute
{
int id;
std::string name;      // "lhs", "rhs", "value", "condition"
PinDirection direction;
PinValueKind valueKind;
DataType type;         // Int, Bool, String, Any
std::string valueBuffer;
};

Examples:

Add node inputs: lhs, rhs
Print node input: value
If node input: condition

This naming is very important because codegen can ask for "lhs" instead of relying on vector index 0 forever.

Third: build graph lookup helpers

Before codegen, write utilities. These will save you.

You want functions like:

const Node* FindNodeById(NodeId id);
const Attribute* FindPinById(PinId id);

const Link* FindLinkEndingAtPin(PinId inputPinId);
std::vector<const Link*> FindLinksStartingAtPin(PinId outputPinId);

const Node* GetNodeConnectedToInputPin(PinId inputPinId);
std::vector<const Node*> GetNodesConnectedFromOutputPin(PinId outputPinId);

const Node* GetNextExecNode(const Node& node);

For data pins:

const Node* GetInputSourceNode(const Node& node, const std::string& pinName);

For field pins:

if not connected, read valueBuffer

These helpers are the foundation.

Fourth: define your actual language model

Do not emit Lua directly from raw graph traversal at first. That becomes messy.

Create tiny IR structs.

Expressions
struct Expr
{
virtual ~Expr() = default;
};

struct IntExpr : Expr
{
int value;
};

struct BoolExpr : Expr
{
bool value;
};

struct StringExpr : Expr
{
std::string value;
};

struct BinaryExpr : Expr
{
std::string op;
std::unique_ptr<Expr> left;
std::unique_ptr<Expr> right;
};

struct VariableExpr : Expr
{
std::string name;
};

struct CallExpr : Expr
{
std::string functionName;
std::vector<std::unique_ptr<Expr>> args;
};
Statements
struct Stmt
{
virtual ~Stmt() = default;
};

struct PrintStmt : Stmt
{
std::unique_ptr<Expr> value;
};

struct ReturnStmt : Stmt
{
std::unique_ptr<Expr> value;
};

struct AssignStmt : Stmt
{
std::string variableName;
std::unique_ptr<Expr> value;
};

struct IfStmt : Stmt
{
std::unique_ptr<Expr> condition;
std::vector<std::unique_ptr<Stmt>> thenBody;
std::vector<std::unique_ptr<Stmt>> elseBody;
};

struct WhileStmt : Stmt
{
std::unique_ptr<Expr> condition;
std::vector<std::unique_ptr<Stmt>> body;
};

struct FunctionStmt : Stmt
{
std::string name;
std::vector<std::string> parameters;
std::vector<std::unique_ptr<Stmt>> body;
};

This is your compiler middle layer.

Fifth: map graph nodes to expression nodes or statement nodes

This is the central rule:

Nodes with execute pins become statements

Examples:

print
return
variable set
if
while
function
Nodes with only data output become expressions

Examples:

int literal
bool literal
add
subtract
comparisons

So write two separate compiler functions:

std::unique_ptr<Expr> CompileExpression(const Node& node);
std::unique_ptr<Stmt> CompileStatement(const Node& node);

This separation will keep the system sane.

How expression compilation works

Suppose you have an Add node.

Its lhs and rhs inputs can come from:

another connected node
a field value typed into the pin

So:

std::unique_ptr<Expr> CompileExpression(const Node& node)
{
switch (node.GetKind())
{
case NodeKind::LiteralInt:
return std::make_unique<IntExpr>(ParseInt(node.GetFieldValue("value")));

        case NodeKind::LiteralBool:
            return std::make_unique<BoolExpr>(ParseBool(node.GetFieldValue("value")));

        case NodeKind::Add:
        {
            auto left = CompileInputExpression(node, "lhs");
            auto right = CompileInputExpression(node, "rhs");

            auto expr = std::make_unique<BinaryExpr>();
            expr->op = "+";
            expr->left = std::move(left);
            expr->right = std::move(right);
            return expr;
        }

        default:
            throw std::runtime_error("Unsupported expression node");
    }
}

And helper:

std::unique_ptr<Expr> CompileInputExpression(const Node& node, const std::string& pinName)
{
const Attribute* pin = node.FindInputPin(pinName);
if (!pin)
throw std::runtime_error("Missing input pin: " + pinName);

    if (auto* sourceNode = GetInputSourceNode(node, pin->id))
        return CompileExpression(*sourceNode);

    if (pin->pinType == PinType::FIELD)
        return MakeLiteralFromField(*pin);

    throw std::runtime_error("Input pin is neither connected nor has field value");
}
How statement compilation works

Example for print:

std::unique_ptr<Stmt> CompileStatement(const Node& node)
{
switch (node.GetKind())
{
case NodeKind::Print:
{
auto stmt = std::make_unique<PrintStmt>();
stmt->value = CompileInputExpression(node, "value");
return stmt;
}

        case NodeKind::Return:
        {
            auto stmt = std::make_unique<ReturnStmt>();
            stmt->value = CompileInputExpression(node, "value");
            return stmt;
        }

        default:
            throw std::runtime_error("Unsupported statement node");
    }
}
Sixth: handle execution chains

A statement graph is not just one node. You need to follow execute flow.

Example:

Function -> Print -> Return

So write something like:

std::vector<std::unique_ptr<Stmt>> CompileExecChain(const Node& startNode)
{
std::vector<std::unique_ptr<Stmt>> statements;

    const Node* current = &startNode;
    std::unordered_set<NodeId> visited;

    while (current)
    {
        if (visited.contains(current->GetId()))
            throw std::runtime_error("Execution cycle detected");

        visited.insert(current->GetId());
        statements.push_back(CompileStatement(*current));
        current = GetNextExecNode(*current);
    }

    return statements;
}

This is for a linear flow.

Later, If and While will branch and contain nested bodies.

Seventh: function node should be treated as entry point

You need entry nodes. Usually:

function nodes
event nodes
maybe start/update nodes later

For now, function node is fine.

A function node should contain:

function name
parameter list
first exec output

Then compile like this:

FunctionStmt CompileFunction(const Node& functionNode)
{
FunctionStmt fn;
fn.name = functionNode.GetFieldValue("name");
fn.parameters = ExtractParameters(functionNode);

    const Node* firstStmt = GetNextExecNode(functionNode);
    if (firstStmt)
        fn.body = CompileExecChain(*firstStmt);

    return fn;
}
Eighth: emit Lua from IR

Once you have IR, Lua emission is easy.

Example:

class LuaEmitter
{
public:
std::string EmitExpr(const Expr& expr);
std::string EmitStmt(const Stmt& stmt, int indent = 0);
std::string EmitFunction(const FunctionStmt& fn);
};
Expressions
std::string LuaEmitter::EmitExpr(const Expr& expr)
{
if (auto p = dynamic_cast<const IntExpr*>(&expr))
return std::to_string(p->value);

    if (auto p = dynamic_cast<const BoolExpr*>(&expr))
        return p->value ? "true" : "false";

    if (auto p = dynamic_cast<const StringExpr*>(&expr))
        return "\"" + EscapeLuaString(p->value) + "\"";

    if (auto p = dynamic_cast<const VariableExpr*>(&expr))
        return p->name;

    if (auto p = dynamic_cast<const BinaryExpr*>(&expr))
        return "(" + EmitExpr(*p->left) + " " + p->op + " " + EmitExpr(*p->right) + ")";

    if (auto p = dynamic_cast<const CallExpr*>(&expr))
    {
        std::string out = p->functionName + "(";
        for (size_t i = 0; i < p->args.size(); ++i)
        {
            if (i > 0) out += ", ";
            out += EmitExpr(*p->args[i]);
        }
        out += ")";
        return out;
    }

    throw std::runtime_error("Unknown expression type");
}
Statements
std::string LuaEmitter::EmitStmt(const Stmt& stmt, int indent)
{
std::string pad(indent * 4, ' ');

    if (auto p = dynamic_cast<const PrintStmt*>(&stmt))
        return pad + "print(" + EmitExpr(*p->value) + ")\n";

    if (auto p = dynamic_cast<const ReturnStmt*>(&stmt))
        return pad + "return " + EmitExpr(*p->value) + "\n";

    if (auto p = dynamic_cast<const AssignStmt*>(&stmt))
        return pad + p->variableName + " = " + EmitExpr(*p->value) + "\n";

    if (auto p = dynamic_cast<const IfStmt*>(&stmt))
    {
        std::string out = pad + "if " + EmitExpr(*p->condition) + " then\n";
        for (const auto& s : p->thenBody)
            out += EmitStmt(*s, indent + 1);

        if (!p->elseBody.empty())
        {
            out += pad + "else\n";
            for (const auto& s : p->elseBody)
                out += EmitStmt(*s, indent + 1);
        }

        out += pad + "end\n";
        return out;
    }

    throw std::runtime_error("Unknown statement type");
}
Function
std::string LuaEmitter::EmitFunction(const FunctionStmt& fn)
{
std::string out = "function " + fn.name + "(";

    for (size_t i = 0; i < fn.parameters.size(); ++i)
    {
        if (i > 0) out += ", ";
        out += fn.parameters[i];
    }

    out += ")\n";

    for (const auto& stmt : fn.body)
        out += EmitStmt(*stmt, 1);

    out += "end\n";
    return out;
}
Ninth: support FIELD pins properly

Your FIELD concept is actually very useful.

For an input pin:

if connected: use connected node output
else if field: parse field buffer directly into literal expr
else: error

Example:

print node input value
connected to add node → compile add expr
not connected, but field says "hello" → compile string literal
neither → validation error

This allows hybrid behavior and is excellent for usability.

Tenth: validation pass before codegen

Do not generate code straight away. First validate.

Checks you want:

Structural validation
every link pin id exists
start pin is output
end pin is input
data pins are connected only to compatible data pins
execute pins are connected only to execute pins
Semantic validation
required input pin is connected or has field value
literal field values can parse correctly
function has valid name
return node only exists inside function
no multiple exec outputs into a single linear pin unless allowed
no cycles in exec graph unless explicitly allowed by while/loop node semantics
Type validation
add expects numeric operands
if expects bool condition
print can accept any type
return matches function return type, if you track that

You do not need full static typing at first, but at least basic compatibility is good.

Eleventh: introduce a tiny type system

You’ll want this soon.

enum class DataType
{
Int,
Bool,
Float,
String,
Void,
Any
};

Every output data pin should say what it produces.
Every input data pin should say what it expects.

That lets validation catch broken graphs early.

Twelfth: decide what subset of Lua you support

Do not try to support “Lua” broadly.

Support your own small graph language that emits Lua.

A good first subset:

Expressions
int literal
bool literal
string literal
variable get
add/sub/mul/div
comparisons
Statements
print
assignment
return
function
if

That is enough to prove the whole pipeline.

Avoid loops and user-defined functions beyond basics until this works.

Suggested immediate roadmap for you
Phase 1

Make these work end-to-end:

Function node
Int literal node
Bool literal node
Add node
Print node
Return node

Example graph:

Function(main)
exec -> Print
Print.value <- Add
Add.lhs <- Int(5)
Add.rhs <- Int(3)
Print.exec -> Return
Return.value <- Int(0)

Generated Lua:

function main()
print((5 + 3))
return 0
end

If you can do this, your architecture is working.

Phase 2

Add:

Variable set
Variable get
Compare nodes
If node

Then support nested blocks.

Phase 3

Add:

function parameters
function calls
while loop
Very important design advice

Do not make every node generate raw strings itself.

That seems tempting:

virtual std::string ToLua();

But it becomes hard to manage indentation, nested statements, validation, reuse, and type handling.

Better:

nodes are editor/runtime graph objects
compiler converts graph to IR
emitter converts IR to Lua

That separation will save you later.

Minimal class responsibilities
Editor layer
Node
Attribute
Link
graph storage
Compiler layer
GraphResolver
GraphValidator
GraphCompiler
Output layer
LuaEmitter

That’s enough.

A simple mental model

Think of your system like this:

[Editor Graph]
↓
[Resolver]
↓
[Validator]
↓
[IR Builder]
↓
[Lua Emitter]
↓
[Lua source code]
One thing you should probably add now

Right now your links store only:

startAttributePinId
endAttributePinId

That’s okay, but make sure you can quickly answer:

which node owns each pin?
what pin name/type is this?
is this exec or data?
is this connected already?

If not, add lookup maps:

std::unordered_map<PinId, Attribute*> pinById;
std::unordered_map<PinId, Node*> pinOwnerById;
std::unordered_multimap<PinId, Link*> linksByStartPin;
std::unordered_map<PinId, Link*> linkByEndPin;

These will make compilation much easier.

If I were you, I would do this next

Implement these 7 functions first:

const Node* GetNextExecNode(const Node& node);
const Node* GetInputSourceNode(const Node& node, const std::string& pinName);
std::string GetFieldValue(const Node& node, const std::string& pinName);

bool ValidateGraph(const Graph& graph, std::vector<std::string>& errors);

std::unique_ptr<Expr> CompileExpression(const Node& node);
std::unique_ptr<Stmt> CompileStatement(const Node& node);
std::string EmitLua(const FunctionStmt& fn);

Once these exist, the rest becomes iteration.

My honest recommendation for your exact setup

Your current base model is already enough. Do not redesign the UI system heavily right now.

Just add:

semantic NodeKind
named pins
pin data types
graph resolver helpers
validator
IR structs
Lua emitter

That is the next real milestone.

If you want, I can sketch a concrete C++ skeleton for:
Node, Attribute, Link, GraphCompiler, Expr/Stmt, and LuaEmitter based directly on your current design.

Right now your compile() is simultaneously acting as:

graph index builder
dependency resolver
execution graph builder
expression resolver
statement emitter
top-level Lua emitter
fallback return handler

That is why it feels muddy. I can already see the right pieces in there: attrInfo, separate dataInputToOutput and execInputToOutput, a data dependency topo sort, and a separate exec traversal. Those are the exact raw ingredients you need.

What is structurally wrong right now

The main issue is this:

you are mixing graph analysis with Lua string generation

For example, resolveInputExpr() is not returning an expression object, it returns Lua text directly, and emitStatement() also decides both semantics and final Lua spelling at the same time. Then later you again emit non-exec nodes in a separate loop, which duplicates logic for nodes like EqualsTo and NotEqualsTo.

So the code already contains the right concepts, but they are currently flattened into one layer.

The structure I recommend

Keep your existing node editor model. Do not throw that away.

Refactor into 4 parts:

1. Graph indexing / lookup

Pure graph facts.

struct PinInfo
{
NodeGraphNode* node = nullptr;
int index = -1;
bool isInput = false;
NodeAttributeType type = NodeAttributeType::PIN;
};

struct GraphIndex
{
std::unordered_map<int, PinInfo> pinInfo;
std::unordered_map<int, int> dataInputToSourcePin;
std::unordered_map<int, int> execInputToSourcePin;

    std::unordered_map<NodeGraphNode*, std::vector<NodeGraphNode*>> execOutgoing;
    std::unordered_map<NodeGraphNode*, int> execIndegree;

    std::unordered_map<NodeGraphNode*, std::unordered_set<NodeGraphNode*>> dataDeps;
    std::unordered_map<NodeGraphNode*, std::vector<NodeGraphNode*>> dataOutgoing;
    std::unordered_map<NodeGraphNode*, int> dataIndegree;

    std::vector<NodeGraphNode*> allNodes;
};

This is basically the cleaner version of what you already built with attrInfo, dataInputToOutput, execInputToOutput, deps, outgoing, and indegree.

2. Resolver helpers

Small functions that answer questions.

Examples:

NodeGraphNode* GetDataSourceNode(const GraphIndex& g, int inputPinId);
NodeGraphNode* GetExecNextNode(const GraphIndex& g, NodeGraphNode* node);
std::vector<NodeGraphNode*> GetExecNextNodes(const GraphIndex& g, NodeGraphNode* node);

const NodeGraphComponents::Node::Attribute* FindInputByName(const NodeGraphNode& node, std::string_view name);
const NodeGraphComponents::Node::Attribute* FindOutputByName(const NodeGraphNode& node, std::string_view name);

This removes repeated map-walking from the emitter.

3. Compiler IR

Do not emit Lua immediately. Convert graph into a tiny internal model first.

Expressions
struct Expr { virtual ~Expr() = default; };

struct NumberExpr : Expr
{
std::string value;
};

struct BoolExpr : Expr
{
bool value = false;
};

struct VariableExpr : Expr
{
std::string name;
};

struct BinaryExpr : Expr
{
std::string op;
std::unique_ptr<Expr> lhs;
std::unique_ptr<Expr> rhs;
};
Statements
struct Stmt { virtual ~Stmt() = default; };

struct PrintStmt : Stmt
{
std::unique_ptr<Expr> value;
};

struct ReturnStmt : Stmt
{
std::unique_ptr<Expr> value;
};

struct FunctionStmt : Stmt
{
std::string name;
std::vector<std::unique_ptr<Stmt>> body;
};

You do not need every node type on day one. Just enough for:

Integer
Boolean
Add
Subtract
GreaterThan
EqualsTo
NotEqualsTo
Print
Return
Function

That already matches what your current code handles.

4. Lua emitter

Once you have Expr and Stmt, converting to Lua becomes simple and dumb.

class LuaEmitter
{
public:
std::string EmitExpr(const Expr& expr);
void EmitStmt(std::ostringstream& out, const Stmt& stmt, int indent);
};

This layer should know nothing about pins, links, topo sort, or node editor IDs.

How your current code maps into the new structure

Here is the clean mapping.

A. buildGraphIndex()

Move all of this into one function:

collect attrInfo
build dataInputToOutput
build execInputToOutput
build data dependency graph
build exec graph

That first half of your current function is already almost exactly this.

B. topoSortDataNodes()

Move the queue / indegree / ordered logic into its own function.

std::vector<NodeGraphNode*> topoSortDataNodes(const GraphIndex& g);

That is your current ready, ordered, and fallback append block.

C. compileExpr(node)

Replace resolveInputExpr() returning std::string with a function returning std::unique_ptr<Expr>.

Right now this function does this:

find data source pin
find source node
if generic object node, emit object.field
otherwise emit nodeVar[srcNode]

That means it is already trying to be an expression compiler — it is just returning strings too early.

A better version is:

std::unique_ptr<Expr> Compiler::CompileExpr(NodeGraphNode* node)
{
const auto& name = node->name();

    if (name == "Integer")
    {
        auto expr = std::make_unique<NumberExpr>();
        expr->value = parseNumberOrDefault(node->outputs()[0].getValueBuff());
        return expr;
    }

    if (name == "Boolean")
    {
        auto expr = std::make_unique<BoolExpr>();
        expr->value = parseBoolean(node->outputs()[0].getValueBuff());
        return expr;
    }

    if (name == "Add")
    {
        auto expr = std::make_unique<BinaryExpr>();
        expr->op = "+";
        expr->lhs = CompileInputExpr(*node, 0);
        expr->rhs = CompileInputExpr(*node, 1);
        return expr;
    }

    // etc...
}
D. compileStmt(node)

Your current emitStatement() should become semantic compilation, not text generation.

Right now emitStatement() both interprets the node and emits Lua lines like:

local node_5 = (...)
print(...)
return ...
local node_3 = function()

Instead:

std::unique_ptr<Stmt> Compiler::CompileStmt(NodeGraphNode* node)
{
const auto& name = node->name();

    if (name == "Print")
    {
        auto stmt = std::make_unique<PrintStmt>();
        stmt->value = CompileInputExpr(*node, 0);
        return stmt;
    }

    if (name == "Return")
    {
        auto stmt = std::make_unique<ReturnStmt>();
        stmt->value = CompileInputExpr(*node, 0);
        return stmt;
    }

    if (name == "Function")
    {
        auto fn = std::make_unique<FunctionStmt>();
        fn->name = "GeneratedFunction"; // later pull from a field
        for (auto* next : graph_.execOutgoing[node])
            fn->body.push_back(CompileExecChain(next));
        return fn;
    }

    return nullptr;
}
E. compileExecChain(startNode)

Your emitExecChain() is conceptually correct, but again, it emits text too soon. It should instead build a list of statements.

The most important redesign rule
Separate these two categories
1. Pure data nodes

These produce values:

Integer
Boolean
Add
Subtract
GreaterThan
EqualsTo
NotEqualsTo

These should compile to Expr.

2. Exec nodes

These define order:

Function
Print
Return

These should compile to Stmt.

Right now your code partly understands this, because it has separate execOutgoing and a special loop for non-exec nodes. That is the right direction.

What I would change first in your code
Step 1: rename your maps to reflect intent

Your current names are workable, but slightly misleading.

Better:

std::unordered_map<int, PinInfo> pinInfoById;
std::unordered_map<int, int> dataSourcePinByInputPin;
std::unordered_map<int, int> execSourcePinByInputPin;

That makes the meaning much clearer than dataInputToOutput.

Step 2: stop using node->name() everywhere

Right now almost all dispatch is string-based:

if (nodeName == "Add") ...
else if (nodeName == "Subtract") ...

That is okay briefly, but brittle.

Add:

enum class NodeKind
{
Integer,
Boolean,
Add,
Subtract,
GreaterThan,
EqualsTo,
NotEqualsTo,
Function,
Print,
Return,
Unsupported
};

Then each node exposes:

virtual NodeKind kind() const = 0;

That alone will make the compiler far easier to maintain.

Step 3: remove duplication for expression nodes

You currently emit EqualsTo and NotEqualsTo in both places:

inside emitStatement()
inside the non-exec node loop later

That duplication is a signal that those nodes really belong in one place: CompileExpr().

Step 4: stop inventing nodeVar for everything

nodeVar[node] = "node_" + std::to_string(node->id()) is fine as a temporary implementation detail.

But in the final structure:

literals should emit literal expressions directly
binary operators should emit (lhs + rhs)
only genuine temporaries should get local variables

Otherwise the generated Lua will become bloated.

For example, instead of:

local node_1 = 5
local node_2 = 3
local node_3 = (node_1 + node_2)
print(node_3)

you can often emit:

print((5 + 3))

That falls out naturally once you compile to IR first.

A concrete target architecture

Here is the exact file split I would use.

GraphIndex.hpp
struct PinInfo;
struct GraphIndex;

GraphIndex BuildGraphIndex(
const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
const std::vector<NodeGraphLink>& links);
GraphQueries.hpp
NodeGraphNode* GetDataSourceNode(const GraphIndex&, int inputPinId);
std::vector<NodeGraphNode*> GetExecNextNodes(const GraphIndex&, NodeGraphNode* node);
FlowScriptIR.hpp
struct Expr;
struct NumberExpr;
struct BoolExpr;
struct BinaryExpr;

struct Stmt;
struct PrintStmt;
struct ReturnStmt;
struct FunctionStmt;
FlowScriptCompiler.hpp
class FlowScriptCompiler
{
public:
std::string Compile(
const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
const std::vector<NodeGraphLink>& links);

private:
GraphIndex graph_;

    std::unique_ptr<Expr> CompileExpr(NodeGraphNode* node);
    std::unique_ptr<Expr> CompileInputExpr(const NodeGraphNode& node, int inputIndex);

    std::unique_ptr<Stmt> CompileStmt(NodeGraphNode* node);
    std::vector<std::unique_ptr<Stmt>> CompileExecChain(NodeGraphNode* start);
};
LuaEmitter.hpp
class LuaEmitter
{
public:
std::string Emit(const std::vector<std::unique_ptr<Stmt>>& stmts);

private:
std::string EmitExpr(const Expr& expr);
void EmitStmt(std::ostringstream& out, const Stmt& stmt, int indent);
};
Minimal migration plan from your current code

Do this in order.

Pass 1

Extract only the graph-building logic from your current function.

Result:

GraphIndex BuildGraphIndex(...);
Pass 2

Extract the data topo sort.

Result:

std::vector<NodeGraphNode*> TopoSortData(const GraphIndex&);
Pass 3

Replace resolveInputExpr() from string-returning to IR-returning.

Pass 4

Replace emitStatement() with CompileStmt().

Pass 5

Add LuaEmitter.

Pass 6

Delete the duplicated non-exec emission loop and the terminalVars fallback return hack, because once you have real function statements, return handling becomes explicit. That current tail-return behavior is a convenience, but it is also mixing “graph result collection” with “function semantics.”

My honest read on your current code

You are closer than you think.

You already discovered the important architectural split:

data graph
exec graph

That is the hardest conceptual part, and your code clearly shows it.

The confusion is coming from one thing only:

you tried to make the graph walker also be the final Lua printer

That is why it feels tangled.

Best next move

Refactor toward this rule:

Graph code answers graph questions. Compiler code builds semantic objects. Emitter code prints Lua.


### LUA Decompilation cleanup

You should think of it as:

Lua parser for your supported visual-script subset

That distinction matters a lot.

Because if you try to support arbitrary Lua like:

for i = 1, 10 do
print(i)
end

or closures, tables, method syntax, local scoping tricks, multiple returns, nested calls, anonymous functions, etc., then Lua → graph becomes a real language frontend.

But if your goal is:

“Take Lua that was generated by my graph compiler, or Lua that follows my small rules, and rebuild nodes/links”

then this becomes very manageable.

The rule I would use
Forward direction

Graph → IR → Lua

Reverse direction

Lua → IR → Graph

So the graph is not compiled directly from Lua text.
And Lua is not parsed directly into nodes and links.

Instead:

Visual Graph <-> IR <-> Lua

That is the correct long-term shape.

Why this is important

Your graph has concepts that plain Lua text does not explicitly preserve:

exact node types
pin identities
field vs connected input
execution pins
layout/position in the editor
whether a literal came from a dedicated Integer node or an inline field
whether an expression was split across several nodes or collapsed

Lua source loses a lot of that information.

Example:

print(5 + 3)

Could map to:

Graph A
Integer(5)
Integer(3)
Add
Print

or

Graph B
Print with field input "5 + 3"

or

Graph C
Precomputed Integer(8)
Print

So reverse mapping is never unique.

That means your reverse compiler must choose a canonical graph form.

What I recommend for decompilation

Use a restricted subset and a canonical reconstruction strategy.

Step 1: define supported Lua subset

Only support things you can represent in your node system.

For now maybe:

Expressions
number literal
boolean literal
variable reference
binary ops: + - * / > == ~=
Statements
print(expr)
return expr
function name() ... end
maybe assignment later
maybe if later

That matches your current compiler direction pretty well. Your current code already recognizes nodes like Integer, Boolean, Add, Subtract, GreaterThan, EqualsTo, NotEqualsTo, Print, Return, and Function.

Do not support arbitrary Lua first.

The reverse pipeline
1. Parse Lua into AST

Not graph yet. Just parse into a small syntax tree.

Example:

struct ExprAst { virtual ~ExprAst() = default; };

struct NumberAst : ExprAst { std::string value; };
struct BoolAst   : ExprAst { bool value; };
struct NameAst   : ExprAst { std::string name; };

struct BinaryAst : ExprAst
{
std::string op;
std::unique_ptr<ExprAst> lhs;
std::unique_ptr<ExprAst> rhs;
};

And for statements:

struct StmtAst { virtual ~StmtAst() = default; };

struct PrintAst : StmtAst
{
std::unique_ptr<ExprAst> value;
};

struct ReturnAst : StmtAst
{
std::unique_ptr<ExprAst> value;
};

struct FunctionAst : StmtAst
{
std::string name;
std::vector<std::unique_ptr<StmtAst>> body;
};

This AST can be the same IR you use for codegen, or very close to it.

2. AST → GraphBuilder

Once you have AST, reconstruct nodes in a consistent way.

Example:

function main()
print(5 + 3)
return 0
end

Becomes:

Function node
Print node
Return node
Add node
Integer node 5
Integer node 3
Integer node 0

Then create links:

Function.execOut → Print.execIn
Print.execOut → Return.execIn
Add.out → Print.value
Int(5).out → Add.lhs
Int(3).out → Add.rhs
Int(0).out → Return.value

That is AST → graph reconstruction.

How to actually structure it

You want three reverse components:

1. Lexer

Turns Lua text into tokens.

Example tokens:

function
end
return
print
identifiers
numbers
booleans
(
)
operators like +, >, ==, ~=
2. Small parser

Turns tokens into AST.

You do not need a general Lua parser if you keep the subset narrow.

3. Graph builder

Turns AST into nodes and links.

Important design choice: canonical node expansion

When rebuilding graph, always choose one canonical representation.

For example:

Literal numbers

Always generate an Integer node.

a + b

Always generate an Add node with two connected inputs.

print(expr)

Always generate a Print node, never inline the whole expr into a field.

That way:

roundtrip is predictable
graph output is consistent
debugging is much easier
Example of reverse building
Lua
print((5 + 3))
AST
PrintAst
value = BinaryAst("+")
lhs = NumberAst("5")
rhs = NumberAst("3")
Graph reconstruction

BuildStmt(PrintAst):

create Print node
call BuildExpr(BinaryAst("+"))

BuildExpr(BinaryAst("+")):

create Add node
recursively build lhs and rhs nodes
link their outputs to Add inputs
return Add output pin

Then connect Add output pin to Print input pin.

That is the whole reverse idea.

A concrete graph builder shape

You will want helper return values like this:

struct GraphValue
{
NodeGraphNode* node = nullptr;
int outputPinId = -1;
};

struct GraphStmt
{
NodeGraphNode* entryExecNode = nullptr;
NodeGraphNode* exitExecNode = nullptr;
};

Why?

Because when you build expressions, you need to know:

which node was created
which output pin carries the resulting value

And when you build statements, you need:

where exec flow starts
where exec flow exits
Expression builder
GraphValue GraphBuilder::BuildExpr(const ExprAst& expr)
{
if (auto n = dynamic_cast<const NumberAst*>(&expr))
{
auto* node = CreateIntegerNode(n->value);
return { node, node->outputs()[0].getId() };
}

    if (auto b = dynamic_cast<const BoolAst*>(&expr))
    {
        auto* node = CreateBooleanNode(b->value);
        return { node, node->outputs()[0].getId() };
    }

    if (auto op = dynamic_cast<const BinaryAst*>(&expr))
    {
        auto* node = CreateBinaryNodeForOp(op->op); // Add / Subtract / EqualsTo etc.

        auto lhs = BuildExpr(*op->lhs);
        auto rhs = BuildExpr(*op->rhs);

        Link(lhs.outputPinId, node->inputs()[0].getId());
        Link(rhs.outputPinId, node->inputs()[1].getId());

        return { node, node->outputs()[0].getId() };
    }

    throw std::runtime_error("Unsupported expr");
}
Statement builder
GraphStmt GraphBuilder::BuildStmt(const StmtAst& stmt)
{
if (auto p = dynamic_cast<const PrintAst*>(&stmt))
{
auto* node = CreatePrintNode();
auto value = BuildExpr(*p->value);

        Link(value.outputPinId, node->inputs()[0].getId());
        return { node, node };
    }

    if (auto r = dynamic_cast<const ReturnAst*>(&stmt))
    {
        auto* node = CreateReturnNode();
        auto value = BuildExpr(*r->value);

        Link(value.outputPinId, node->inputs()[0].getId());
        return { node, node };
    }

    throw std::runtime_error("Unsupported stmt");
}
Function builder
NodeGraphNode* GraphBuilder::BuildFunction(const FunctionAst& fn)
{
auto* fnNode = CreateFunctionNode(fn.name);

    NodeGraphNode* previousExec = fnNode;

    for (const auto& stmt : fn.body)
    {
        auto built = BuildStmt(*stmt);

        Link(previousExec->getExecOutput()->getId(),
             built.entryExecNode->getExecInput()->getId());

        previousExec = built.exitExecNode;
    }

    return fnNode;
}
What parser style should you use

Since you said “simple system without ANTLR,” I would use a handwritten recursive descent parser.

That is perfectly fine for a small subset.

You need:

Tokens
enum class TokenType
{
Identifier,
Number,
True,
False,
Function,
End,
Return,
Print,
LParen,
RParen,
Plus,
Minus,
Star,
Slash,
Greater,
EqualEqual,
NotEqual,
Comma,
Eof
};
Parser functions
std::unique_ptr<FunctionAst> ParseFunction();
std::unique_ptr<StmtAst> ParseStatement();
std::unique_ptr<ExprAst> ParseExpression();
std::unique_ptr<ExprAst> ParseEquality();
std::unique_ptr<ExprAst> ParseComparison();
std::unique_ptr<ExprAst> ParseTerm();
std::unique_ptr<ExprAst> ParseFactor();
std::unique_ptr<ExprAst> ParsePrimary();

That is enough to parse arithmetic and simple statements.

Operator precedence

Do not parse binary expressions in one flat function.
Use precedence levels.

For example:

equality: ==, ~=
comparison: >
term: +, -
factor: *, /
primary: literals, identifiers, parenthesized expr

That way:

5 + 3 * 2

becomes:

Add(
5,
Multiply(3, 2)
)

not

Multiply(
Add(5, 3),
2
)
The hardest part in reverse direction

Not parsing.

The hardest part is deciding what to do with Lua features that do not map neatly to your visual graph.

Examples:

local declarations
table constructors
method calls
loops
multiple assignment
nested function definitions
generic Lua function calls other than known builtins
unary operators
string literals if you don’t yet have string nodes
scoping

So I would explicitly define:

Supported for import
functions with straight-line bodies
print
return
numeric/bool expressions
known binary ops
Not supported yet
everything else

And when unsupported syntax appears:

stop import
report a precise error
maybe highlight the token or statement
The roundtrip truth

The realistic goal is not:

any Lua can become graph

The realistic goal is:

graph-generated Lua can roundtrip back into a graph

That is much more achievable.

So your import system should target this standard first:

Graph -> Lua -> Graph

Once that works well, you can slowly accept more hand-written Lua.

Best practical strategy
Phase 1

Only import Lua that your own compiler emits.

That means:

predictable syntax
known node kinds
known function structure
no surprise constructs

This is the easiest and most valuable.

Phase 2

Allow nicely written equivalent Lua within your supported subset.

Example:

print(5 + 3)
return 4
function main() ... end
Phase 3

Add more language features as you add node types.

One more recommendation: preserve metadata in emitted Lua

If roundtrip matters a lot, you can optionally emit comments.

Example:

--@node:Function
function main()
--@node:Print
print((5 + 3))
--@node:Return
return 0
end

or even:

--@graph-hint op=Add
print((5 + 3))

Then the importer can use hints when present, but still work without them.

This is optional, but it can make roundtripping much more stable.

My recommendation for your engine

Do this:

Compiler side

Keep building:

Graph → IR → Lua
Import side

Build:

Lua lexer
Lua subset parser
AST → graph builder

And use the same IR node types for both directions.

That is the cleanest architecture.