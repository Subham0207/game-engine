#pragma once
#include <Controls/statemachine.hpp>
#include <Helpers/Shared.hpp>
#include <EngineState.hpp>

Controls::ToStateWhenCondition::ToStateWhenCondition(std::shared_ptr<State> state, std::function<bool()> condition)
{
    this->state = state;
    this->condition = condition;
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
        auto playeThisState = activeState->toStateWhenCondition->at(i).condition();
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
        std::ifstream ifs(filename);
        boost::archive::text_iarchive ia(ifs);
        ia >> *this;
}