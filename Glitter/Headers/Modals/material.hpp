#pragma once
#include <Modals/texture.hpp>

namespace Modals
{
    struct Material
    {
        ProjectModals::Texture* albedo;
        ProjectModals::Texture* normal;
        ProjectModals::Texture* metalness;
        ProjectModals::Texture* roughness;
        ProjectModals::Texture* ao;

        Material()=default;

        private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & albedo;
            ar & normal;
            ar & metalness;
            ar & roughness;
            ar & ao;
        }
    };
}