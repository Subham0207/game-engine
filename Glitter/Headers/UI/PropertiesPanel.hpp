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

            void PropertiesPanel::setPointLight(PointLight* pointLight);
            void PropertiesPanel::setDirectionalLight(DirectionalLight* dl);
            void PropertiesPanel::setSpotLight(SpotLight* sl);

        private:
            void resetAll();
    };
}