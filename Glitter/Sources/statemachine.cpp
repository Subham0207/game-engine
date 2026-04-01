#pragma once
#include <Controls/statemachine.hpp>
#include <Helpers/Shared.hpp>
#include <EngineState.hpp>
#include <NodeGraph/StateMachineJsonExporter.hpp>

#include <unordered_map>

Controls::ToStateWhenCondition::ToStateWhenCondition(std::shared_ptr<State> state, std::string condition)
{
    this->state = state;
    this->luaCondition = LuaCondition{condition};
    this->cppCondition = []{return false;};
}

Controls::ToStateWhenCondition::ToStateWhenCondition(std::shared_ptr<State> state, std::function<bool()> condition)
{
    this->state = state;
    this->cppCondition = condition;
}

Controls::State::State(std::string stateName)
{
    this->stateName = stateName;
    toStateWhenCondition = std::vector<ToStateWhenCondition>();
    animation = NULL;
    blendspace = NULL;
}

bool Controls::State::Play(Animator* animator, glm::vec2 scrubLoc)
{
    if(!animationGuid.empty())
    {
        animator->SetLoopCurrentAnimation(animationShouldLoop);
        animator->PlayAnimation(animation);

        if (!animationShouldLoop && animation)
        {
            auto duration = animation->GetDuration();
            if(animator->m_ElapsedTime >= duration)
            {
                animator->m_ElapsedTime = duration;

                if (animNotify)
                    animNotify();

                return true;
            }
        }

        return false;
    }

    if(!blendspaceGuid.empty())
    {
        blendspace->interpolateToScrubberLocation(scrubLoc);
        auto blendSelection = blendspace->GetBlendSelection();
        // std::cout <<"[statemachine][play] scrubLocL: " << blendspace->getScrubberLocation().x << " scrubLocY: " << blendspace->getScrubberLocation().y << std::endl;
        // std::cout << "[statemachine][play] blendfactors: " <<
        //     "bottomLeft: " << blendSelection->bottomLeftBlendFactor << " "
        //     "bottomRight: " << blendSelection->bottomRightBlendFactor << " "
        //     "bottomTopLeft: " << blendSelection->topLeftBlendFactor << " "
        //     "bottomTopRight: " << blendSelection->topRightBlendFactor << std::endl;
        animator->PlayAnimationBlended(blendSelection);
    }

    return false;
}

void Controls::State::assignBlendspace(BlendSpace2D* blendspace)
{
    this->blendspaceGuid =  blendspace->getAssetId();
    this->blendspace = blendspace;
}
void Controls::State::assignAnimation(Animation* animation)
{
    this->animationGuid = animation->getAssetId();
    this->animation = animation;
}

void Controls::State::assignAnimation(std::string animationGuid, bool noLoop, std::function<void()> animNotify)
{
    this->animationGuid = animationGuid;
    this->animation = Animation::loadAnimation(animationGuid);
    this->animationShouldLoop = !noLoop;
    this->noLoop = noLoop;
    this->animNotify = animNotify;
}

Controls::StateMachine::StateMachine(std::string filename): Serializable()
{
    stateGraph = NULL;
    activeState = NULL;
    this->filename = filename;
};

void Controls::StateMachine::tick(Animator* animator)
{
    if (!started)
    {
        this->onStart();
        started = true;
    }

    if(!activeState)
        return;

    this->onTick();

    auto tryTransitionOnce = [&]() -> bool
    {
        for (size_t i = 0; i < activeState->toStateWhenCondition.size(); i++)
        {
            auto playThisState = evaluateLuaCondition(activeState->toStateWhenCondition[i].luaCondition);
            if(playThisState)
            {
                activeState = activeState->toStateWhenCondition[i].state;
                activeState->animationCompletionApplied = false;
                std::cerr << "[StateMachine] Enter state='" << activeState->stateName
                          << "' animLoop=" << (activeState->animationShouldLoop ? "true" : "false")
                          << " completionField='" << activeState->animationCompletionBoolField
                          << "' completionValue=" << (activeState->animationCompletionBoolValue ? "true" : "false")
                          << std::endl;
                animator->initNoLoopAnimation();
                return true;
            }
        }

        return false;
    };

    // Order matters:
    // 1) Try normal transition conditions.
    // 2) Play current state.
    // 3) If one-shot completion mutates context, re-evaluate transitions immediately.
    tryTransitionOnce();

    //2. then set blendselection and m_currentAnimation
    const glm::vec2 scrubLoc = evaluateBlendspaceScrub(*activeState);
    const bool completedNow = activeState->Play(animator, scrubLoc);
    if (completedNow && !activeState->animationCompletionApplied)
    {
        const bool applied = applyAnimationCompletion(*activeState);
        std::cerr << "[StateMachine] Animation completed once. state='" << activeState->stateName
                  << "' loop=" << (activeState->animationShouldLoop ? "true" : "false")
                  << " field='" << activeState->animationCompletionBoolField
                  << "' value=" << (activeState->animationCompletionBoolValue ? "true" : "false")
                  << " applied=" << (applied ? "true" : "false")
                  << std::endl;
        activeState->animationCompletionApplied = true;

        // Re-check transitions immediately after completion side-effects.
        // This avoids waiting one extra frame and avoids missing short-lived flags.
        tryTransitionOnce();
    }
    //3. not here but executes: it is the actual poseTransition logic
}

bool Controls::StateMachine::evaluateLuaCondition(LuaCondition& condition) const
{
    if (!m_luaEvalWithContext || m_luaEvalContext == nullptr)
        return false;

    try
    {
        return m_luaEvalWithContext(condition, getLuaEngine(), m_luaEvalContext);
    }
    catch (const std::exception& ex)
    {
        std::cerr << "[StateMachine] Lua condition eval failed: " << ex.what() << "\n";
        return false;
    }
}

glm::vec2 Controls::StateMachine::evaluateBlendspaceScrub(const State& state) const
{
    if (!m_blendspaceScrubWithContext || m_luaEvalContext == nullptr)
        return glm::vec2(0.0f, 0.0f);

    return m_blendspaceScrubWithContext(state, m_luaEvalContext);
}

bool Controls::StateMachine::applyAnimationCompletion(const State& state)
{
    if (!m_applyAnimationCompletionWithContext || m_luaEvalContext == nullptr)
        return false;

    if (state.animationCompletionBoolField.empty())
    {
        std::cerr << "[StateMachine] Completion skip: no bool field selected for state='"
                  << state.stateName << "'" << std::endl;
        return false;
    }

    return m_applyAnimationCompletionWithContext(state, m_luaEvalContext);
}

void Controls::StateMachine::setActiveState(std::shared_ptr<State> state)
{
    this->stateGraph = state;
    this->activeState = state;
}

void Controls::StateMachine::LoadSMfile(std::string filename)
{
    std::vector<StateMachineNode> graphNodes;
    std::vector<StateMachineLink> graphLinks;
    int rootNodeId = -1;

    if (!StateMachineJsonExporter::DeserializeChainJson(filename, graphNodes, graphLinks, rootNodeId))
    {
        std::cerr << "[StateMachine] Failed to load .sm file: " << filename << "\n";
        return;
    }

    // Build runtime states first so links can resolve targets by node id.
    std::unordered_map<int, std::shared_ptr<State>> statesById;
    statesById.reserve(graphNodes.size());
    for (const auto& node : graphNodes)
    {
        auto runtimeState = std::make_shared<State>(node.name);
        if (!node.resourceGuid.empty())
        {
            if (node.type == StateMachineNodeType::Animation)
                runtimeState->animationGuid = node.resourceGuid;
            else if (node.type == StateMachineNodeType::Blendspace)
                runtimeState->blendspaceGuid = node.resourceGuid;
        }

        runtimeState->blendspaceAxisXField = node.blendspaceAxisXField;
        runtimeState->blendspaceAxisYField = node.blendspaceAxisYField;
        runtimeState->animationShouldLoop = node.animationShouldLoop;
        runtimeState->animationCompletionBoolField = node.animationCompletionBoolField;
        runtimeState->animationCompletionBoolValue = node.animationCompletionBoolValue;
        runtimeState->animationCompletionApplied = false;

        statesById[node.id] = runtimeState;
    }

    // Wire transitions. Conditions from .sm map directly to luaCondition.
    for (const auto& link : graphLinks)
    {
        auto fromIt = statesById.find(link.fromNodeId);
        auto toIt = statesById.find(link.toNodeId);
        if (fromIt == statesById.end() || toIt == statesById.end())
            continue;

        fromIt->second->toStateWhenCondition.emplace_back(toIt->second, link.condition);
    }

    auto rootIt = statesById.find(rootNodeId);
    if (rootIt == statesById.end())
    {
        std::cerr << "[StateMachine] Root state not found in .sm graph: " << filename << "\n";
        stateGraph = nullptr;
        activeState = nullptr;
        states.clear();
        return;
    }

    stateGraph = rootIt->second;
    activeState = rootIt->second;
    activeState->animationCompletionApplied = false;
    this->filename = fs::path(filename).filename().stem().string();

    // Resolve animation/blendspace pointers and rebuild cached state list/index mapping.
    auto filesMap = getEngineRegistryFilesMap();
    traverseAndLoadStateGraph(activeState, filesMap);
}

void Controls::StateMachine::saveContent(fs::path contentFile, std::ostream& os)
{
    // we don't save animation and blendspace again because they are is thier own entity. So should be already saved.

    //save the statemachine
    fs::path dir = fs::path(contentFile.string()).parent_path();
    if (dir.empty()) {
        // Set the directory to the current working directory
        dir = fs::current_path();
    }
    if (!fs::exists(dir)) {
        if (!fs::create_directories(dir)) {
            std::cerr << "Failed to create directories: " << dir << std::endl;
            return;
        }
    }
    std::ofstream ofs(contentFile.string());
    boost::archive::text_oarchive oa(ofs);
    oa << *this;
    ofs.close();
}

void Controls::StateMachine::loadContent(fs::path contentFile, std::istream& is)
{
        std::ifstream ifs(contentFile.string());
        boost::archive::text_iarchive ia(ifs);
        ia >> *this;

        //states in state graph are loaded but the animation will need to repointed correctly.
        auto filesMap = getEngineRegistryFilesMap();
        stateGraph = activeState;
        traverseAndLoadStateGraph(activeState, filesMap);

        //Add to EngineState
        EngineState::state->statemachines.push_back(this);
}

void Controls::StateMachine::traverseAndLoadStateGraph(std::shared_ptr<State> state, std::map<std::string, std::string> filesMap)
{
    std::unordered_set<State*> visited;           // or unordered_set<std::string> if you have state->guid
    dfsLoad(activeState, filesMap, visited);
    states = std::vector<State*>(visited.begin(), visited.end());

    for (auto &&i: states)
    {
        for (auto &&j : i->toStateWhenCondition)
        {
            auto statename = j.state->stateName;
            for (size_t k = 0; k < states.size(); k++)
            {
                if(states[k]->stateName == statename)
                j.index = k;
            }
            
        }
        
    }
    
}

void Controls::StateMachine::dfsLoad(const std::shared_ptr<State>& state,
    std::map<std::string, std::string>& filesMap,
    std::unordered_set<State*>& visited)
{
        if (!state) return;

    // Stop if we've already seen this node (prevents infinite loops on cycles)
    auto [_, inserted] = visited.insert(state.get());
    if (!inserted) return;

    // Load animation once
    if (!state->animationGuid.empty()) {
        if (auto it = filesMap.find(state->animationGuid); it != filesMap.end()) {
            auto p = fs::path(it->second);
            auto parent = p.parent_path();
            state->animation = new Animation();
            state->animation->load(parent, state->animationGuid);
        } else {
            std::cerr << "[StateMachine] No file for animationGuid " << state->animationGuid << "\n";
        }
    }

    // Load blendspace once
    if (!state->blendspaceGuid.empty()) {
        if (auto it = filesMap.find(state->blendspaceGuid); it != filesMap.end()) {
            auto p = fs::path(it->second);
            auto parent = p.parent_path();
            state->blendspace = new BlendSpace2D();
            state->blendspace->load(parent, state->blendspaceGuid);
        } else {
            std::cerr << "[StateMachine] No file for blendspaceGuid " << state->blendspaceGuid << "\n";
        }
    }

    // Recurse through outgoing edges
    auto size = state->toStateWhenCondition.size();
    if (size > 0) {
        for (const auto& edge : state->toStateWhenCondition) {
            dfsLoad(edge.state, filesMap, visited);
        }
    }
}