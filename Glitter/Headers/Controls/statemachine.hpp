#pragma once
#include <3DModel/Animation/Animation.hpp>
#include <3DModel/Animation/Animator.hpp>
#include <Controls/BlendSpace2D.hpp>
#include <vector>
#include <functional>
#include <serializeAClass.hpp>
#include <Serializable.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <LuaEngine/LuaCondition.hpp>
#include <unordered_set>
#include <type_traits>

#include <Helpers/NodeGraphHelpers.hpp>
#include <boost/pfr.hpp>

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
        std::string blendspaceAxisXField;
        std::string blendspaceAxisYField;

        State()=default;
        State(std::string stateName);
        void Play(Animator* animator, glm::vec2 scrubLoc = glm::vec2(0.0f));
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
            
            const std::string typeName() const override {return "statemachine"; }

            void LoadSMfile(std::string filename);

            template<typename T>
            void loadContext(T& t)
            {
                m_luaEvalContext = static_cast<const void*>(&t);
                m_luaEvalWithContext = [](LuaCondition& condition, LuaEngine& engine, const void* ctx) -> bool
                {
                    const auto& typedCtx = *static_cast<const T*>(ctx);
                    sol::table luaContext = StateMachine::template buildLuaContextTable<T>(engine, typedCtx);
                    return condition.evaluate(engine, luaContext);
                };

                m_blendspaceScrubWithContext = [](const State& state, const void* ctx) -> glm::vec2
                {
                    const auto& typedCtx = *static_cast<const T*>(ctx);
                    return glm::vec2(
                        StateMachine::template getContextFloatFieldByName<T>(typedCtx, state.blendspaceAxisXField),
                        StateMachine::template getContextFloatFieldByName<T>(typedCtx, state.blendspaceAxisYField)
                    );
                };
            }

            void clearContext()
            {
                m_luaEvalContext = nullptr;
                m_luaEvalWithContext = {};
                m_blendspaceScrubWithContext = {};
            }
        protected:
            //TODO: Remove saving and loading this object to disk. We now load sm file saved by statemachinegraph.
            virtual void saveContent(fs::path contentFileLocation, std::ostream& os) override;
            virtual void loadContent(fs::path contentFileLocation, std::istream& is) override;
        private:
            void traverseAndLoadStateGraph(std::shared_ptr<State> state, std::map<std::string, std::string> filesMap);

            void dfsLoad(const std::shared_ptr<State>& state,
            std::map<std::string, std::string>& filesMap,
            std::unordered_set<State*>& visited);

            std::shared_ptr<State> stateGraph;
            std::shared_ptr<State> activeState;
            std::string filename;

            bool started = false;

            const void* m_luaEvalContext = nullptr;
            std::function<bool(LuaCondition&, LuaEngine&, const void*)> m_luaEvalWithContext;
            std::function<glm::vec2(const State&, const void*)> m_blendspaceScrubWithContext;
            bool evaluateLuaCondition(LuaCondition& condition) const;
            glm::vec2 evaluateBlendspaceScrub(const State& state) const;

            template<typename T>
            static float getContextFloatFieldByName(const T& context, const std::string& fieldName)
            {
                if (fieldName.empty())
                    return 0.0f;

                const auto fieldNames = NodeGraphHelpers::get_field_names<T>();
                float resolvedValue = 0.0f;
                bool found = false;
                std::size_t currentIndex = 0;

                boost::pfr::for_each_field(context, [&](const auto& field)
                {
                    if (found)
                    {
                        ++currentIndex;
                        return;
                    }

                    if (currentIndex < fieldNames.size() && fieldNames[currentIndex] == fieldName)
                    {
                        using FieldType = std::decay_t<decltype(field)>;
                        if constexpr (std::is_same_v<FieldType, float>)
                            resolvedValue = field;
                        found = true;
                    }

                    ++currentIndex;
                });

                return resolvedValue;
            }

            template<typename T>
            static sol::table buildLuaContextTable(LuaEngine& engine, const T& context)
            {
                sol::table table = engine.state().create_table();
                const auto fieldNames = NodeGraphHelpers::get_field_names<T>();

                std::size_t currentIndex = 0;
                boost::pfr::for_each_field(context, [&](const auto& field)
                {
                    if (currentIndex >= fieldNames.size())
                    {
                        ++currentIndex;
                        return;
                    }

                    const std::string& fieldName = fieldNames[currentIndex];
                    using FieldType = std::decay_t<decltype(field)>;

                    if constexpr (std::is_same_v<FieldType, bool>
                               || std::is_integral_v<FieldType>
                               || std::is_floating_point_v<FieldType>
                               || std::is_same_v<FieldType, std::string>)
                    {
                        table[fieldName] = field;
                    }
                    else if constexpr (std::is_same_v<FieldType, const char*> || std::is_same_v<FieldType, char*>)
                    {
                        table[fieldName] = field ? field : "";
                    }

                    ++currentIndex;
                });

                return table;
            }

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & stateGraph;
                ar & activeState;
                ar & filename;
            }
    };

}