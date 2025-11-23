#pragma once

class SpotLight;
class PointLight;
class DirectionalLight;

namespace UI
{
    class PropertiesPanel
    {
        public:
            PropertiesPanel();
            void draw();
            SpotLight *spotlight;
            PointLight *pointLight;
            DirectionalLight *directionalLight;
        private:
    };
}