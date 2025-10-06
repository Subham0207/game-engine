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
    if(animation)
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

    if(blendspace)
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

Controls::StateMachine::StateMachine()
{
    stateGraph = NULL;
    activeState = NULL;
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
        traverseAndLoadstateGraph(currentTraversedState, filesMap);
}

void Controls::StateMachine::traverseAndLoadstateGraph(std::shared_ptr<State> state, std::map<std::string, std::string> filesMap)
{
    if(!state)
    return;

    if(!state->animationGuid.empty())
    {
        auto animationGuid = state->animationGuid;
        auto animation_location = fs::path(filesMap[animationGuid]);
        state->animation = new Animation();
        state->animation->load(animation_location.parent_path(), animationGuid);
    }
    
    if(!state->blendspaceGuid.empty())    
    {
        auto blendspaceGuid = state->blendspaceGuid;
        auto blendspace_location = fs::path(filesMap[blendspaceGuid]);
        state->blendspace = new BlendSpace2D();
        state->blendspace->load(blendspace_location.parent_path(), blendspaceGuid);
    }

    for (size_t i = 0; i < state->toStateWhenCondition->size(); i++)
    {
        traverseAndLoadstateGraph(state->toStateWhenCondition->at(i).state, filesMap);
    }
}