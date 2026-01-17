//
// Created by subha on 17-01-2026.
//

#ifndef GLITTER_EVENTBUS_HPP
#define GLITTER_EVENTBUS_HPP

#include <functional>
#include <Event/Event.hpp>

class EventBus
{
public:
    template<typename T>
    using Fn = std::function<void(const T&)>;

    template<typename T>
    void subscribe(Fn<T> fn)
    {
        static_assert(std::is_base_of_v<Event, T>, "T must derive from Event");

        auto wrapper = [f = std::move(fn)](const Event& e)
        {
            f(static_cast<const T&>(e));
        };

        m_handlers[eventTypeOf<T>()].push_back(std::move(wrapper));
    }

    void dispatch(const Event& e) const {
        auto it = m_handlers.find(e.type());
        if (it == m_handlers.end()) return;

        for (auto& h : it->second) {
            h(e);
            // if you want "stop propagation":
            // if (e.handled) break;
        }
    }

private:
    using Handler = std::function<void(const Event&)>;
    std::unordered_map<EventType, std::vector<Handler>> m_handlers;

    template<typename T>
    static EventType eventTypeOf();
};

template<> inline EventType EventBus::eventTypeOf<MouseMoveEvent>() { return EventType::MouseMove; }

#endif //GLITTER_EVENTBUS_HPP