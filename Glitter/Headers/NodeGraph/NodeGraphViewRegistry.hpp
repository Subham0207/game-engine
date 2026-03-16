//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODEGRAPH_VIEWREGISTRY_HPP
#define GLITTER_NODEGRAPH_VIEWREGISTRY_HPP

#include <algorithm>
#include <memory>
#include <vector>

#include "INodeGraphView.hpp"
#include "NodeGraphRenderContext.hpp"

class NodeGraphViewRegistry
{
public:
    template <typename T, typename... Args>
    T& emplaceView(Args&&... args)
    {
        auto v = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *v;
        m_views.emplace_back(std::move(v));
        m_sorted = false;
        return ref;
    }

    void drawAll(NodeGraphRenderContext& ctx)
    {
        sortIfNeeded();
        for (auto& v : m_views)
            v->draw(ctx);
    }

    template <typename T>
    T* findView()
    {
        for (auto& v : m_views)
        {
            if (auto* p = dynamic_cast<T*>(v.get()))
                return p;
        }
        return nullptr;
    }

private:
    void sortIfNeeded()
    {
        if (m_sorted)
            return;
        std::stable_sort(
            m_views.begin(),
            m_views.end(),
            [](const std::unique_ptr<INodeGraphView>& a, const std::unique_ptr<INodeGraphView>& b) {
                if ((int)a->layer() != (int)b->layer())
                    return (int)a->layer() < (int)b->layer();
                return a->priority() < b->priority();
            });
        m_sorted = true;
    }

    std::vector<std::unique_ptr<INodeGraphView>> m_views;
    bool m_sorted = true;
};

#endif //GLITTER_NODEGRAPH_VIEWREGISTRY_HPP

