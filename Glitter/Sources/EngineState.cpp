#include <EngineState.hpp>
#include <utility>

#include "ProjectManifest.hpp"
#include "Camera/FlyCam.hpp"
#include "Helpers/GetExecutablePath.hpp"
#include "Helpers/Shared.hpp"

EngineState::EngineState(){
    state = nullptr;
}

void EngineState::init()
{
    bus = EventBus();
    ais = std::vector<AI::AI*>();
    mEditorWindow = nullptr;
    mStatemachineWindow = nullptr;

    auto manifestPath = GetExecutablePath::getExecutableDir().string() + "/" + "Project.manifest.json";
    projectManifest = new ProjectManifest(manifestPath);

    auto cwd = GetExecutablePath::getExecutableDir().string();
    isDevMode = projectManifest->isDevelopment();
    auto enginePath = isDevMode ? projectManifest->getEngineDir(): cwd;
    auto projectDir = isDevMode ? projectManifest->getProjectDir(): cwd;
    std::cout << "Is Development mode: " << isDevMode << std::endl;
    std::cout << "CWD: " << cwd << std::endl;
    setEngineDirectory(enginePath);
    setCurrentActiveProjectDir(projectDir);

    editorCamera = new FlyCam("editorCamera");
    bus.subscribe<MouseMoveEvent>([&](const MouseMoveEvent& e)
    {
        editorCamera->onMouseMove(e);
    });
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
    getUIState().metalicTextureID = Shared::generateMetallicTexture();
    getUIState().nonMetalicTextureID = Shared::generateNonMetallicTexture();
    getUIState().whiteAOTextureID = Shared::generateWhiteAOTexture();
    // Default roughness texture used when a mesh has no material assigned.
    unsigned int defaultRoughnessTextureID = Shared::generateDefaultRoughnessTexture();
    getUIState().flatNormalTextureID= Shared::generateFlatNormalTexture();

    auto vertPath = fs::path(engineInstalledDirectory) / "Shaders/pbr.vert";
    auto fragPath = fs::path(engineInstalledDirectory) / "Shaders/pbr.frag";
    defaultMaterial = std::make_shared<Materials::Material>("DefaultMaterial", vertPath.string(), fragPath.string());
    auto& textureUnits = defaultMaterial->GetTextureUnits();
    textureUnits.albedo->id = getUIState().nonMetalicTextureID;
    textureUnits.normal->id = getUIState().flatNormalTextureID;
    textureUnits.metalness->id = getUIState().nonMetalicTextureID;
    textureUnits.roughness->id = defaultRoughnessTextureID;
    textureUnits.ao->id = getUIState().whiteAOTextureID;
    defaultMaterialInstance = std::make_shared<Materials::MaterialInstance>("defaultMaterialInstance", defaultMaterial) ;
}
