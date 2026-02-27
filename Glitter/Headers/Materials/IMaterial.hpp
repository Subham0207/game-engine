//
// Created by subha on 26-02-2026.
//

#ifndef GLITTER_IMATERIAL_HPP
#define GLITTER_IMATERIAL_HPP
#include "Serializable.hpp"
#include "TextureUnits.hpp"
#include "Helpers/shader.hpp"

namespace Materials
{
    class IMaterial: public Serializable {
    public:
        virtual ~IMaterial() = default;
        virtual void Bind() = 0; // The Mesh calls this before drawing
        [[nodiscard]] virtual Shader* GetShader() const = 0;
        virtual TextureUnits& GetTextureUnits() = 0;
    };
}

#endif //GLITTER_IMATERIAL_HPP