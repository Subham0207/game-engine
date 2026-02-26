//
// Created by subha on 26-02-2026.
//

#ifndef GLITTER_TEXTUREUNITS_HPP
#define GLITTER_TEXTUREUNITS_HPP
#include "Modals/texture.hpp"
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>


namespace Materials
{
    class TextureUnits
    {
    public:
        TextureUnits();
        ~TextureUnits();

        static void BindTextures(const TextureUnits& textureUnits);

        std::shared_ptr<ProjectModals::Texture> albedo;
        std::shared_ptr<ProjectModals::Texture> normal;
        std::shared_ptr<ProjectModals::Texture> metalness;
        std::shared_ptr<ProjectModals::Texture> roughness;
        std::shared_ptr<ProjectModals::Texture> ao;
    private:
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


#endif //GLITTER_TEXTUREUNITS_HPP