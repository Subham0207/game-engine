#include <EngineState.hpp>
#include <cstdlib>
#include <utility>

EngineState::EngineState(){
    ais = std::vector<AI::AI*>();
    mWindow = nullptr;

    auto value = std::getenv("GLITTER_ENGINE");
    engineInstalledDirectory
     = value != nullptr ? value : fs::current_path().string();

    currentActiveProjectDirectory = "";
}

void EngineState::setEngineDirectory(std::string value)
{
    engineInstalledDirectory = std::move(value);
}

void EngineState::setCurrentActiveProjectDir(std::string value)
{
    currentActiveProjectDirectory = std::move(value);
}

fs::path EngineState::navIntoProjectDir(std::string path)
{
    auto filePath = fs::path(state->currentActiveProjectDirectory) / path;
    auto parentPath = filePath.parent_path();
    if (!fs::exists(parentPath)) {
            fs::create_directories(parentPath);
    }
    return filePath;
}

fs::path EngineState::navIntoEnginDir(std::string path)
{
    return fs::path(state->engineInstalledDirectory) / path;
}

EngineState* EngineState::state = nullptr;

void EngineState::GenerateDefaultMaterials()
{
    auto vertPath = fs::path(engineInstalledDirectory) / "Shaders/staticShader.vert";
    auto fragPath = fs::path(engineInstalledDirectory) / "Shaders/staticShader.frag";
    defaultMaterial = std::make_shared<Materials::Material>("DefaultMaterial", vertPath.string(), fragPath.string());
    auto& textureUnits = defaultMaterial->GetTextureUnits();
    textureUnits.albedo->id = getUIState().nonMetalicTextureID;
    textureUnits.normal->id = getUIState().flatNormalTextureID;
    textureUnits.metalness->id = getUIState().nonMetalicTextureID;
    textureUnits.roughness->id = getUIState().whiteAOTextureID;
    textureUnits.ao->id = getUIState().whiteAOTextureID;
    defaultMaterialInstance = std::make_shared<Materials::MaterialInstance>("defaultMaterialInstance", defaultMaterial) ;
}
