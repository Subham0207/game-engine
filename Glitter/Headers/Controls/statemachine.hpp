#pragma once
#include <3DModel/Animation/Animation.hpp>
#include <3DModel/Animation/Animator.hpp>
#include <Controls/PlayerController.hpp>
#include <Controls/BlendSpace2D.hpp>
#include <vector>
#include <functional>
#include <serializeAClass.hpp>
#include <Serializable.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <LuaEngine/LuaCondition.hpp>
#include <unordered_set>

namespace Controls
{
    struct State;
    struct ToStateWhenCondition
    {
        std::shared_ptr<State> state = NULL;
        int index;
        LuaCondition condition;

        ToStateWhenCondition()=default;
        ToStateWhenCondition(std::shared_ptr<State> state, std::string condition);

        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & state;
                ar & condition;
            }
    };

    struct State
    {
        std::string stateName = "";
        std::vector<ToStateWhenCondition> toStateWhenCondition;
        
        std::string animationGuid;
        Animation* animation;
        
        std::string blendspaceGuid;
        BlendSpace2D* blendspace;
        
        State()=default;
        State(std::string stateName);
        void Play(std::shared_ptr<Controls::PlayerController> playerController, Animator* animator);
        void assignBlendspace(BlendSpace2D* blendspace);
        void assignAnimation(Animation* animation);
        void assignAnimation(std::string animationGuid);

        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & stateName;
                ar & toStateWhenCondition;
                ar & animationGuid;
                ar & blendspaceGuid;
            }
    }; 

    class StateMachine: public Serializable {
        public:
            StateMachine()=default;
            StateMachine(std::string filename);

            virtual void onStart(){};
            virtual void onTick(){};
            virtual void onDestroy(){};

            void tick(std::shared_ptr<Controls::PlayerController> playerController, Animator* animator);
            void setActiveState(std::shared_ptr<State> state);
            std::shared_ptr<State> getActiveState() {return activeState;};
            std::shared_ptr<State> getStateGraph() {return stateGraph;}
            virtual const std::string contentName() override {return filename; }
            void setFileName(std::string name){ this->filename = name;}
            std::vector<State*> states;

            std::string GetClassId() const override { return "StateMachine"; }
            
        protected:
            virtual const std::string typeName() const override {return "statemachine"; }
            
            virtual void saveContent(fs::path contentFileLocation, std::ostream& os) override;
            virtual void loadContent(fs::path contentFileLocation, std::istream& is) override;
        private:
            void traverseAndLoadStateGraph(std::shared_ptr<State> state, std::map<std::string, std::string> filesMap);

            void StateMachine::dfsLoad(const std::shared_ptr<State>& state,
            std::map<std::string, std::string>& filesMap,
            std::unordered_set<State*>& visited);

            std::shared_ptr<State> stateGraph;
            std::shared_ptr<State> activeState;
            std::string filename;

            bool started = false;
            
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & stateGraph;
                ar & activeState;
                ar & filename;
            }
    };

}