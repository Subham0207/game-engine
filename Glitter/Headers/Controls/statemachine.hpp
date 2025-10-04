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

namespace Controls
{
    struct State;
    struct ToStateWhenCondition
    {
        std::shared_ptr<State> state = NULL;
        std::function<bool()> condition;

        ToStateWhenCondition(std::shared_ptr<State> state, std::function<bool()> condition);

        template <typename Owner>
        void rebind(Owner* owner) {
            condition = buildPredicate(owner, spec);
        }

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
        std::string stateName;
        std::vector<ToStateWhenCondition>* toStateWhenCondition;

        std::string animationGuid;
        Animation* animation;

        std::string blendspaceGuid;
        BlendSpace2D* blendspace;

        State(std::string stateName);
        void Play(Controls::PlayerController* playerController, Animator* animator);

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
            StateMachine();
            void tick(Controls::PlayerController* playerController, Animator* animator);
            void setActiveState(std::shared_ptr<State> state);

            
        protected:
            virtual const std::string typeName() const override {return "character"; }
            virtual const std::string contentName() override {return filename; }
            
            virtual void saveContent(fs::path contentFileLocation, std::ostream& os) override;
            virtual void loadContent(fs::path contentFileLocation, std::istream& is) override;
        private:
            std::shared_ptr<State> stateGraph;
            std::shared_ptr<State> activeState;
            std::string filename;
            
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & stateGraph;
                ar & activeState;
                ar & filename;
            }
    };

}