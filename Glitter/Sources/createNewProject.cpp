#include <Helpers/createNewProject.hpp>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <Level/Level.hpp>
#include <Physics/box.hpp>
#include <EngineState.hpp>
#include <Helpers/Shared.hpp>
#include <AI/AI.hpp>

void write_text(const fs::path& p, const std::string& text)
{
    fs::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::binary);
    f << text;
}

int openAProject(const std::string& currentDir)
{
    EngineState::state->currentActiveProjectDirectory = currentDir;
    
    // start the editor.

    //load the main lvl from this project.
    return 0;
}

void update_recent_projects_list(const fs::path& projects_file_path, const fs::path& new_project_path) {
    fs::path absolute_new_path = fs::absolute(new_project_path);

    // 2. Add the new project path to the top of the list.
    // First, remove any existing entry for this project to avoid duplicates.
    getUIState().recent_projects.erase(
        std::remove_if(getUIState().recent_projects.begin(), getUIState().recent_projects.end(),
            [&](const fs::path& p) { return fs::absolute(p) == absolute_new_path; }),
        getUIState().recent_projects.end());

    // Insert the new project at the beginning of the vector.
    getUIState().recent_projects.insert(getUIState().recent_projects.begin(), new_project_path);

    // 3. Trim the list to the maximum size.
    if (getUIState().recent_projects.size() > MAX_PROJECTS) {
        getUIState().recent_projects.resize(MAX_PROJECTS);
    }

    // 4. Write the updated list back to the file.
    // The `std::ofstream` constructor with `std::ios::trunc` will overwrite the file.
    std::ofstream outfile(projects_file_path, std::ios::trunc);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << projects_file_path << std::endl;
        return;
    }
    for (const auto& path : getUIState().recent_projects) {
        outfile << path.string() << std::endl;
    }
    outfile.close();

    std::cout << "Successfully updated recent projects file: " << projects_file_path << std::endl;
}

Controls::StateMachine* setupStateMachine(fs::path projectAssetDirectory)
{
    //animations
    auto rootAnimationPath = fs::path("E:/OpenGL/Models/Animations");
    auto animFilePaths = std::vector<fs::path>
    {
        rootAnimationPath / "Breathing Idle.fbx",
        rootAnimationPath / "Standard Walk.fbx" ,
        rootAnimationPath / "Running.fbx",
        rootAnimationPath / "Jump.fbx",
        rootAnimationPath / "Jog Strafe Left.fbx",
        rootAnimationPath / "Jog Strafe Right.fbx",
        rootAnimationPath / "Walking Backwards.fbx",
        rootAnimationPath / "Falling Idle.fbx",
        rootAnimationPath / "Sprinting Forward Roll.fbx"
    };

    auto animations = new std::vector<Animation*>();
    for (size_t i = 0; i < animFilePaths.size(); i++)
    {
        auto animation = new Animation(animFilePaths[i].string());
        animation->save(projectAssetDirectory);
        animations->push_back(animation);
    }

    //state machine 
    auto animStateMachine = new Controls::StateMachine("characterMovements");
    auto locomotionState = std::make_shared<Controls::State>("Locomotion");
    auto jumpState = std::make_shared<Controls::State>("Jump");
    auto dodgeRollState = std::make_shared<Controls::State>("DodgeRoll");

    locomotionState->toStateWhenCondition.push_back(
        Controls::ToStateWhenCondition(
        jumpState,
        R"(
            return function(playerController)
                return playerController ~= nil and (not playerController.grounded)
        end
        )")
    );
    
    jumpState->toStateWhenCondition.push_back(
        Controls::ToStateWhenCondition(
        locomotionState,
        R"(
            return function(playerController)
                return playerController ~= nil and (playerController.grounded)
        end
        )")
    );

    locomotionState->toStateWhenCondition.push_back(
        Controls::ToStateWhenCondition(
        dodgeRollState,
        R"(
            return function(playerController)
                return playerController ~= nil and (playerController.dodgeStart)
        end
        )")
    );

    dodgeRollState->toStateWhenCondition.push_back(
        Controls::ToStateWhenCondition(
        locomotionState,
        R"(
            return function(playerController)
                return playerController ~= nil and (not playerController.dodgeStart)
        end
        )")
    );

    animStateMachine->setActiveState(locomotionState);

    locomotionState->assignBlendspace(new BlendSpace2D("character_blendspace"));

    locomotionState->blendspace->AddBlendPoint(glm::vec2(0.0f, 0.0f), animations->at(0));
    locomotionState->blendspace->AddBlendPoint(glm::vec2(-1.0f, 0.0f), animations->at(4));
    locomotionState->blendspace->AddBlendPoint(glm::vec2(1.0f, 0.0f), animations->at(5));

    locomotionState->blendspace->AddBlendPoint(glm::vec2(0.0f, 1.0f), animations->at(1));
    locomotionState->blendspace->AddBlendPoint(glm::vec2(-1.0f, 1.0f), animations->at(1));
    locomotionState->blendspace->AddBlendPoint(glm::vec2(1.0f, 1.0f), animations->at(1));

    locomotionState->blendspace->AddBlendPoint(glm::vec2(0.0f, 2.0f), animations->at(2));
    locomotionState->blendspace->AddBlendPoint(glm::vec2(-1.0f, 2.0f), animations->at(2));
    locomotionState->blendspace->AddBlendPoint(glm::vec2(1.0f, 2.0f), animations->at(2));

    locomotionState->blendspace->AddBlendPoint(glm::vec2(-1.0f, -1.0f), animations->at(6));
    locomotionState->blendspace->AddBlendPoint(glm::vec2(0.0f, -1.0f), animations->at(6));
    locomotionState->blendspace->AddBlendPoint(glm::vec2(1.0f, -1.0f), animations->at(6));

    locomotionState->blendspace->save(projectAssetDirectory);

    jumpState->assignAnimation(animations->at(7));
    dodgeRollState->assignAnimation(animations->at(8));
    return animStateMachine;
}

void create_cmake_game_project(const std::string& projectDir, const std::string& projectName)
{
    fs::path root = fs::path(projectDir) / projectName;

    //add cmakelist.txt
    // using a "Template File" Approach via simple search-and-replace function to fill it out.

    std::ifstream in(fs::path("./Template/cmakelist_project_template.txt").string());
    std::ostringstream buffer;
    buffer << in.rdbuf();

    std::string content = buffer.str();

    size_t pos;
    while ((pos = content.find("{{PROJECT_NAME}}")) != std::string::npos) {
        content.replace(pos, 16, projectName);
    }

    fs::path cmakelistfile_loc = root / "CMakeLists.txt"s;
    std::ofstream out(cmakelistfile_loc);
    out << content;

    //cmake --install E:\OpenGL\game-engine\cmake-build-debug-visual-studio --config Debug --prefix E:/opengl/Bins/glitterEngineBincmake
    // (DONE) create a package out of your engine using cmake --install build --config Debug --prefix E:/opengl/Bins/glitterEngineBincmake. [Later this figure out changes to have minimum accessors exposed in cmake]
    // Then find_package cmd that we add to project's cmake can start using the engine lib/DLL files.
    // update cmakelist.txt of engine to support install cmd.

    fs::path source = "./Template/main.txt";
    fs::path target = root / "main.cpp";

    try {
        fs::copy(source, target);
        fs::create_directory(root / "src");
        fs::create_directory(root / "src/headers");
        fs::create_directory(root / "src/sources");
    }
    catch (fs::filesystem_error& e) {
        std::cerr << "Error creating main.cpp : " << e.what() << std::endl;
    }
}

int create_new_project(const std::string& currentDir, const std::string& projectName)
{
    fs::path root = fs::path(currentDir) / projectName;
    if (fs::exists(root)) {
        std::cerr << "Target path exists: " << root << "\n";
        return 1;
    }

    auto projectAssetDirectory = root/ "Assets";

    auto projectId = boost::uuids::to_string(boost::uuids::random_generator()()); 

    // Folders
    fs::create_directories(root / "Assets");
    fs::create_directories(root / "Levels");
    fs::create_directories(root / "Config");
    fs::create_directories(root / "Library"); // cache/imports (ignore in VCS)

    EngineState::state->currentActiveProjectDirectory = root.string();


    create_cmake_game_project(currentDir, projectName);

    // Manifest (keep it tiny; add fields as you grow)
    auto lvl = new Level();

    auto floorBox = std::make_shared<Model>("./EngineAssets/cube.fbx");
    floorBox->setTransform(glm::vec3(0.0f),glm::quat(), glm::vec3(100.0f,1.0f,100.0f));
    floorBox->attachPhysicsObject(new Physics::Box(&getPhysicsSystem(), false, true));
    floorBox->save(root/ "Assets");
    lvl->addRenderable(floorBox);
    
    auto character = addPlayableCharacter(root, projectAssetDirectory);
    lvl->addRenderable(character);
    
    character->generateInstanceGuid(); // essentially means a new instance of character.
    lvl->addRenderable(character);

    auto ai = addAICharacter(root, character);
    lvl->addAI(ai);
    lvl->save(root / "Levels");

    std::string manifest = std::string(R"({
        "engineVersion": "0.1.0",
        "projectName": ")") + projectName + R"(",
        "projectId": ")" + projectId + R"(",
        "mounts": {
            "assets": "Assets/",
            "scenes": "Levels/",
            "config": "Config/",
            "cache": "Library/"
        },
        "defaultLevel": "Levels/)" + lvl->GetGuid() + R"(",
        "plugins": []
        }
        )";

    write_text(root / "Project.manifest.json", manifest);

    GenerateLuaLSConfig(root);

    update_recent_projects_list(fs::path(EngineState::state->engineInstalledDirectory) / "user_prefs.json", root);

    delete lvl;

    std::cout << "Created project '" << projectName << "' at " << root << "\n";
    std::cout << "Project ID: " << projectId << "\n";
    return 0;
}

std::shared_ptr<Character> addPlayableCharacter(std::filesystem::path root, std::filesystem::path projectAssetDirectory)
{
    auto character = std::make_shared<Character>("./EngineAssets/Aj.fbx");
    character->model->setTransform(glm::vec3(0.0f,3.0f,0.0f),glm::quat(), glm::vec3(0.03f,0.03,0.03));
    character->capsuleColliderPosRelative = glm::vec3(0.0f,-2.5f,0.0f);

    character->animStateMachine = setupStateMachine(projectAssetDirectory);

    character->save(root/ "Assets");   
    return character;
}

AI::AI* addAICharacter(std::filesystem::path root, std::shared_ptr<Character> aiCharacter)
{
    //NOTE: We are using the same character only to save so there are no two instances of samething on disk. And we get 1 guid or character.
    //While loading the AI -- be sure to load another instance of the character using its guid.
    aiCharacter->model->setTransform(glm::vec3(0.0f,3.0f,1.0f),glm::quat(), glm::vec3(0.03f,0.03,0.03));
    auto playerController = aiCharacter->playerController;
    // aiCharacter->capsuleCollider->halfHeight = 2.0f;
    // getActiveLevel().addRenderable(aiCharacter);
    auto ai = new AI::AI(aiCharacter, "defaultAI");
    ai->save(root / "Assets");
    // getUIState().ai = ai;
    return ai;
}

void GenerateLuaLSConfig(const fs::path& projectRoot)
{
    const fs::path luarcPath = projectRoot / ".luarc.json";

    // Use relative paths so the folder works when moved/zipped.
    const std::string luarc = R"JSON(
{
  "runtime": {
    "version": "Lua 5.4",
    "path": [
      "?.lua",
      "?/init.lua",
      "Assets/Scripts/?.lua",
      "Assets/Scripts/?/init.lua"
    ]
  },
  "workspace": {
    "library": [
      "Assets/Scripts",
      "EngineAPI"
    ],
    "checkThirdParty": false
  },
  "diagnostics": {
    "globals": ["Engine", "Scene", "Input", "Time", "Log"]
  },
  "hint": {
    "enable": true
  }
}
)JSON";

    Shared::WriteTextFile(luarcPath, luarc);

    // Optional: also ensure stub folder exists
    fs::create_directories(projectRoot / "EngineAPI");
    fs::create_directories(projectRoot / "Assets/Scripts");
}