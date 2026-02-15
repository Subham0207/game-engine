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
        LuaCondition luaCondition;
        std::function<bool()> cppCondition = []{return false;};

        ToStateWhenCondition()=default;
        ToStateWhenCondition(std::shared_ptr<State> state, std::string condition);
        ToStateWhenCondition(std::shared_ptr<State> state, std::function<bool()> condition);

        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & state;
                ar & luaCondition;
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
        void Play(Animator* animator);
        void assignBlendspace(BlendSpace2D* blendspace);
        void assignAnimation(Animation* animation);
        void assignAnimation(std::string animationGuid, bool noLoop, std::function<void()> animNotify);

        //only used in case of animation;
        bool noLoop = false;
        std::function<void()> animNotify;
        // ---------------------

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

            void tick(Animator* animator);
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