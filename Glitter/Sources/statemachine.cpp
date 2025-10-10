#pragma once
#include <Controls/statemachine.hpp>
#include <Helpers/Shared.hpp>
#include <EngineState.hpp>

Controls::ToStateWhenCondition::ToStateWhenCondition(std::shared_ptr<State> state, std::string condition)
{
    this->state = state;
    this->condition = LuaCondition{condition};
}

Controls::State::State(std::string stateName)
{
    this->stateName = stateName;
    toStateWhenCondition = new std::vector<ToStateWhenCondition>();
    animation = NULL;
    blendspace = NULL;
}

void Controls::State::Play(Controls::PlayerController* playerController, Animator* animator)
{
    if(!animationGuid.empty())
    {
        animator->PlayAnimation(animation);
        // Logic to excecute animation only once -- AnimNotify
        auto duration = animation->GetDuration();
        if(stateName == "DodgeRoll" && animator->m_ElapsedTime > duration)
        {
            playerController->dodgeStart = false;
            animator->m_ElapsedTime = 0.0f;
        }
        return;
    }

    if(!blendspaceGuid.empty())
    {
        float xfactor = playerController->movementDirection;
        float yfactor = playerController->movementSpeed;
        
        auto blendSelection = blendspace->GetBlendSelection(glm::vec2(xfactor, yfactor));
        animator->PlayAnimationBlended(blendSelection);
    }
}

void Controls::State::assignBlendspace(BlendSpace2D* blendspace)
{
    this->blendspaceGuid =  blendspace->getGUID();
    this->blendspace = blendspace;
}
void Controls::State::assignAnimation(Animation* animation)
{
    this->animationGuid = animation->getGUID();
    this->animation = animation;
}

Controls::StateMachine::StateMachine(std::string filename)
{
    stateGraph = NULL;
    activeState = NULL;
    this->filename = filename;
};

void Controls::StateMachine::tick(Controls::PlayerController* playerController, Animator* animator)
{
    if(!activeState)
    return;

    //Order of execution here is very importatant to correctly apply pose transition
    //1. first setPoseTransition bool if present.
    for (size_t i = 0; i < activeState->toStateWhenCondition->size(); i++)
    {
        auto playeThisState = activeState->toStateWhenCondition->at(i).condition.evaluate(getLuaEngine());
        if(playeThisState)
        {
            activeState = activeState->toStateWhenCondition->at(i).state;
            animator->initNoLoopAnimation();
            break;
        }
    }

    //2. then set blendselection and m_currentAnimation
    activeState->Play(playerController, animator);
    //3. not here but executes: it is the actual poseTransition logic
}

void Controls::StateMachine::setActiveState(std::shared_ptr<State> state)
{
    this->activeState = state;
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
        auto currentTraversedState = activeState;
        traverseAndLoadStateGraph(currentTraversedState, filesMap);
}

void Controls::StateMachine::traverseAndLoadStateGraph(std::shared_ptr<State> state, std::map<std::string, std::string> filesMap)
{
    std::unordered_set<const State*> visited;           // or unordered_set<std::string> if you have state->guid
    dfsLoad(activeState, filesMap, visited);
}

void Controls::StateMachine::dfsLoad(const std::shared_ptr<State>& state,
    std::map<std::string, std::string>& filesMap,
    std::unordered_set<const State*>& visited)
{
        if (!state) return;

    // Stop if we've already seen this node (prevents infinite loops on cycles)
    auto [_, inserted] = visited.insert(state.get());
    if (!inserted) return;

    // Load animation once
    if (!state->animationGuid.empty()) {
        if (auto it = filesMap.find(state->animationGuid); it != filesMap.end()) {
            auto p = fs::path(it->second);
            state->animation = new Animation();
            state->animation->load(p.parent_path(), state->animationGuid);
        } else {
            std::cerr << "[StateMachine] No file for animationGuid " << state->animationGuid << "\n";
        }
    }

    // Load blendspace once
    if (!state->blendspaceGuid.empty()) {
        if (auto it = filesMap.find(state->blendspaceGuid); it != filesMap.end()) {
            auto p = fs::path(it->second);
            state->blendspace = new BlendSpace2D();
            state->blendspace->load(p.parent_path(), state->blendspaceGuid);
        } else {
            std::cerr << "[StateMachine] No file for blendspaceGuid " << state->blendspaceGuid << "\n";
        }
    }

    // Recurse through outgoing edges
    if (state->toStateWhenCondition) {
        for (const auto& edge : *state->toStateWhenCondition) {
            dfsLoad(edge.state, filesMap, visited);
        }
    }
}