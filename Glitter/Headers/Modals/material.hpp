#pragma once
#include <Modals/texture.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>

namespace Modals
{
    struct Material
    {
        std::shared_ptr<ProjectModals::Texture> albedo;
        std::shared_ptr<ProjectModals::Texture> normal;
        std::shared_ptr<ProjectModals::Texture> metalness;
        std::shared_ptr<ProjectModals::Texture> roughness;
        std::shared_ptr<ProjectModals::Texture> ao;

        Material()
        {
            albedo = std::make_shared<ProjectModals::Texture>();
            normal = std::make_shared<ProjectModals::Texture>();
            metalness = std::make_shared<ProjectModals::Texture>();
            roughness = std::make_shared<ProjectModals::Texture>();
            ao = std::make_shared<ProjectModals::Texture>();
        }

        virtual ~Material()
        {
            albedo.reset();
            normal.reset();
            metalness.reset();
            roughness.reset();
            ao.reset();
        }

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