//
// Created by subha on 17-01-2026.
//

#ifndef GLITTER_EVENTQUEUE_HPP
#define GLITTER_EVENTQUEUE_HPP
#include <mutex>
#include <queue>
#include <type_traits>

#include "Event.hpp"


class EventQueue
{
public:
    template<typename T, typename ...Args>
    void push(Args&&... args)
    {
        static_assert(std::is_base_of_v<Event, T>, "T musts derive from Event");
        std::lock_guard<std::mutex> lock(m_mtx);
        m_q.push(std::make_unique<T>(std::forward<Args>(args)...));
    }

    template<typename F>
    void drain(F&& fn)
    {
        std::queue<std::unique_ptr<Event>> local;
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            std::swap(local, m_q);
        }

        while (!local.empty())
        {
            fn(*local.front());
            local.pop();
        }
    }

private:
    std::mutex m_mtx;
    std::queue<std::unique_ptr<Event>> m_q;
};


#endif //GLITTER_EVENTQUEUE_HPP