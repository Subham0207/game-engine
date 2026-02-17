#include  <Level/Level.hpp>
#include <EngineState.hpp>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <AI/NavMeshBuilder.hpp>
#include <Modals/3DModelType.hpp>
#include <AI/AI.hpp>

#include "GenericFactory.hpp"
#include "Prefab.hpp"
#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_io.hpp"
namespace bs = boost::property_tree;
namespace fs = std::filesystem;

Level::Level(): Serializable()
{
    AIs = std::vector<std::shared_ptr<AI::AI>>();
}

void Level::loadMainLevelOfCurrentProject()
{
    // read the project.manifest file and find the which level is entry point
    auto filesMap = EngineState::state->engineRegistry->renderableSaveFileMap;
    auto manifestDir = fs::path(EngineState::state->currentActiveProjectDirectory) / "project.manifest.json";
    bs::ptree manifest;
    bs::read_json(manifestDir.string(), manifest);
    auto defaultLevelFilePath = fs::path(manifest.get<std::string>("defaultLevel"));
    auto parentPath = defaultLevelFilePath.parent_path();
    auto filename = defaultLevelFilePath.filename().stem().string(); // filename without extension
    //load the level
    this->load(parentPath, filename);
}

void Level::loadContent(fs::path contentFile, std::istream& is)
{
    auto filesMap = EngineState::state->engineRegistry->renderableSaveFileMap;
    if(filesMap.empty())
    return;

    try {
        bs::ptree levelContent;
        bs::read_json(contentFile.string(), levelContent);

        // Iterate over "renderables" array
        const bs::ptree& renderablesNode = levelContent.get_child("renderables");
        for (const auto& kv : renderablesNode)
        {
            const std::string instanceId = kv.first;
            const bs::ptree &renderable = kv.second;

            // Read the id
            std::string id = renderable.get<std::string>("assetId");

            const bs::ptree& tr = renderable.get_child("transform");

            glm::vec3 t = readVec3(tr, "t");
            glm::quat q = readQuat(tr, "r");
            glm::vec3 s = readVec3(tr, "s");

            // Build matrix: T * R * S
            glm::mat4 M = glm::translate(glm::mat4(1.0f), t)
                        * glm::toMat4(q)
                        * glm::scale(glm::mat4(1.0f), s);

            auto contentFilePath = fs::path(filesMap[id]);
            std::string extension = contentFilePath.extension().string();
            auto path =contentFilePath.parent_path();

            if(extension ==  ".model")
            {
                auto model = std::make_shared<Model>();
                model->load(path, id);
                model->setModelMatrix(M);
                model->setInstanceId(instanceId);
                auto filename = contentFilePath.stem().filename().string();
                model->setFileName(filename);
                this->renderables.push_back(model);
                instanceIdToSerializableMap[instanceId] = model;
            }
            if(extension ==  ".character")
            {
                auto character = std::make_shared<Character>();
                character->load(path, id);
                character->setModelMatrix(M);
                character->setInstanceId(instanceId);
                this->renderables.push_back(character);
                instanceIdToSerializableMap[instanceId] = character;
            }
            if (extension == ".characterprefab")
            {
                auto character = spawnCharacter(contentFilePath, M, instanceId);
                character->setAssetId(id);
                instanceIdToSerializableMap[instanceId] = character;

                if(auto pathVariablesNode = renderable.get_child_optional("patrol_variables"))
                {
                    if (auto aiController = std::dynamic_pointer_cast<AI::AI>(character->controller))
                    {
                        auto& variables = aiController->getVariables();

                        for (auto& [key, tree] : *pathVariablesNode)
                        {
                            glm::vec3 pos;

                            // Get the x, y, z values.
                            // Using get<float> ensures the string in the JSON is converted correctly.
                            pos.x = tree.get<float>("x", 0.0f); // 0.0f is a default value if 'x' is missing
                            pos.y = tree.get<float>("y", 0.0f);
                            pos.z = tree.get<float>("z", 0.0f);

                            variables.push_back(pos);
                        }
                    }
                }
            }
        }

        const bs::ptree& aisNode = levelContent.get_child("ais");
        for (const auto& kv : aisNode) {
            const std::string aiEntityId = kv.first;   // "ai-001"
            const bs::ptree& aiNode = kv.second;

            std::string id = aiNode.get<std::string>("assetId");

            auto aiContentFilePath = fs::path(filesMap[id]);
            auto ai = std::make_shared<AI::AI>();
            auto path = aiContentFilePath.parent_path();
            ai->load(path, id);
            ai->setInstanceId(aiEntityId);

            this->AIs.push_back(ai);
        }

    } catch (const bs::ptree_error &e) {
        std::cerr << "Error reading JSON: " << e.what() << "\n";
    }
}

void Level::setupLevelVertices(std::vector<float> navVerts, std::vector<unsigned int> navTris)
{
    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
    auto vertShader = engineFSPath / "Shaders/debug/debug.vert";
    auto fragShader = engineFSPath / "Shaders/debug/debug.frag";
    lvlVerticesShader = new Shader(vertShader.u8string().c_str(),fragShader.u8string().c_str());
    lvlVerticesShader->use();
    lvlVerticesMesh = new Mesh();
    for (size_t i = 0; i < navVerts.size(); i += 3) {
        ProjectModals::Vertex vertex;
        vertex.Position.x = navVerts[i];
        vertex.Position.y = navVerts[i+1];
        vertex.Position.z = navVerts[i+2];
        lvlVerticesMesh->vertices.push_back(vertex);
    }
    lvlVerticesMesh->indices = navTris;
    lvlVerticesMesh->setupMesh();
}

void Level::renderLevelvertices(Camera *camera)
{
    if(verts.size() == 0) return;
    lvlVerticesShader->use();
    auto view = camera->viewMatrix();
    auto projection = camera->projectionMatrix();
    glm::mat4 mvp = projection * view * glm::mat4(1.0f);  
    lvlVerticesShader->setMat4("uMVP", mvp);
    lvlVerticesShader->setVec3("uColor", glm::vec3(0.0f, 0.3f, 0.0f));
    lvlVerticesMesh->Draw(lvlVerticesShader);
}

void Level::renderDebugNavMesh(Camera *camera)
{
    debugNavMeshShader->use();
    auto view = camera->viewMatrix();
    auto projection = camera->projectionMatrix();
    glm::mat4 mvp = projection * view * glm::mat4(1.0f);  
    debugNavMeshShader->setMat4("uMVP", mvp);
    debugNavMeshShader->setVec3("uColor", glm::vec3(0.0f, 0.3f, 0.0f));
    debugNavMesh->Draw(debugNavMeshShader);
}

void Level::tickAIs(float deltaTime)
{
    for(auto ai: AIs)
    {
        ai->OnTick(deltaTime);
    }
}

void Level::createACopyForRenderableAt(int index)
{
    auto renderable = renderables.at(index);
    if (auto model = std::dynamic_pointer_cast<Model>(renderable))
    {
        auto modelCopy = std::make_shared<Model>(*model);
        addRenderable(modelCopy);
    }
}

shared_ptr<Character> Level::spawnCharacter(fs::path actualFilePath, glm::mat4 transform, std::string instanceId)
{
    CharacterPrefabConfig characterPrefab;
    Engine::Prefab::readCharacterPrefab(actualFilePath, characterPrefab);

    auto character = CharacterFactory::Create(characterPrefab.classId);

    character->filename = actualFilePath.filename().stem().string();
    character->setWorldTransform(transform);
    if (instanceId.empty())
    {
        character->generateInstanceGuid();
    }
    else
    {
        character->setInstanceId(std::move(instanceId));
    }
    character->setScale(characterPrefab.modelScale);

    character->animator = new Animator();

    auto model = new Model();
    auto modelParentPath = fs::path(getEngineRegistryFilesMap()[characterPrefab.modelGuid]).parent_path();

    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
    auto vertPath = engineFSPath / "Shaders/basic.vert";
    auto fragPath = engineFSPath / "Shaders/pbr.frag";
    auto shader =  new Shader(vertPath.u8string().c_str(),fragPath.u8string().c_str());
    model->shader = shader;

    model->load(modelParentPath, characterPrefab.modelGuid);
    character->model = model;

    auto skeleton = new Skeleton::Skeleton();
    auto skeletonParentPath = fs::path(getEngineRegistryFilesMap()[characterPrefab.skeletonGuid]).parent_path();
    skeleton->load(skeletonParentPath, characterPrefab.skeletonGuid);
    character->skeleton = skeleton;

    character->animStateMachine = StateMachineFactory::Create(characterPrefab.stateMachineClassId);

    //TODO: make this playerController a scriptable file.
    character->controller = ControllerFactory::Create(characterPrefab.controllerClassId);
    if (auto playerController = std::dynamic_pointer_cast<Controls::PlayerController>(character->controller))
    {
        EngineState::state->playerControllers.push_back(playerController);
        playerController->setCharacter(character);
    }
    if (auto ai = std::dynamic_pointer_cast<AI::AI>(character->controller))
    {
        addAI(ai);
    }

    character->capsuleCollider = new Physics::Capsule(&getPhysicsSystem(),characterPrefab.capsuleRadius, characterPrefab.capsuleHalfHeight, true, true);
    character->modelRelativePosition = characterPrefab.modelRelativePosition;

    character->camera = new Camera("charactercamera");
    character->camera->cameraPos = model->GetPosition();
    float pitchAngle = 0.3f;
    glm::quat pitchQuat = glm::angleAxis(pitchAngle, glm::vec3(1, 0, 0));
    glm::quat newRot = pitchQuat * model->GetRot();
    character->camera->cameraFront = glm::rotate(newRot, glm::vec3(0.0f, 0.0f, 1.0f));
    character->camera->cameraUp = glm::rotate(newRot, glm::vec3(0.0f, 1.0f, 0.0f));
    cameras.push_back(character->camera);

    addRenderable(character);

    return character;
}

void Level::spawnAI(fs::path filepath, std::string instanceId)
{
}

void Level::addAI(std::shared_ptr<AI::AI> ai)
{
    AIs.push_back(ai);
}

void Level::BuildLevelNavMesh()
{
    int baseVert = 0;

    for(int i=0;i<renderables.size();i++)
    {
        auto r = renderables.at(i);

        if (!r->ShouldRender()) continue;
        if(r->getModelType() == ModelType::LIGHT) continue;

        // Get world-space vertices/indices from your mesh
        std::vector<ProjectModals::Vertex> meshVerts;
        std::vector<unsigned int> meshIndices;
        r->BuildFlattenedGeometry(meshVerts, meshIndices);

        // Append verts
        for (const auto& v : meshVerts)
        {
            verts.push_back(v.Position.x);
            verts.push_back(v.Position.y);
            verts.push_back(v.Position.z);
        }

        // Append tris (offset by baseVert)
        for (size_t j = 0; j < meshIndices.size(); ++j)
        {
            tris.push_back(baseVert + (int)meshIndices[j]);
        }

        baseVert += (int)meshVerts.size();
    }

    NavMeshConfig cfg;
    builder = new NavMeshBuilder();

    setupLevelVertices(verts, tris); // Here we get vertices from all models in the level and convert into 1 mesh. And that mesh is used to build th nav mesh.
    std::vector<int> i_vec(tris.begin(), tris.end());
    bool ok = builder->build(verts.data(), (int)verts.size()/3,
                            i_vec.data(), (int)tris.size()/3,
                            cfg);

    if (ok)
    {
        navMesh = builder->getNavMesh();
        navQuery = builder->getNavMeshQuery();

        auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
        auto vertPath = engineFSPath / "Shaders/debug/debug.vert";
        auto fragPath = engineFSPath / "Shaders/debug/debug.frag";
        debugNavMeshShader = new Shader(vertPath.u8string().c_str(),fragPath.u8string().c_str());
        debugNavMeshShader->use();
        std::vector<float> outVerts;
        std::vector<unsigned> outTris;
        builder->getDebugNavmesh(
            outVerts,
            outTris
        );
        debugNavMesh = new Mesh();
        for (size_t i = 0; i < outVerts.size(); i += 3) {
            ProjectModals::Vertex vertex;
            vertex.Position.x = outVerts[i];
            vertex.Position.y = outVerts[i+1];
            vertex.Position.z = outVerts[i+2];
            debugNavMesh->vertices.push_back(vertex);
        }
        debugNavMesh->indices = outTris;
        debugNavMesh->setupMesh();
    }
}

bool Level::FindPath(
              const float* startPos,
              const float* endPos,
              std::vector<float>& outPath)
{
    if (!navQuery) return false;

    dtQueryFilter filter;
    filter.setIncludeFlags(0x01); // our walkable flag
    filter.setExcludeFlags(0);

    float polyPickExt[3] = { 2.0f, 4.0f, 2.0f }; // search box around point

    dtPolyRef startRef, endRef;
    float nearestStart[3], nearestEnd[3];

    navQuery->findNearestPoly(startPos, polyPickExt, &filter, &startRef, nearestStart);
    navQuery->findNearestPoly(endPos, polyPickExt, &filter, &endRef, nearestEnd);

    if (!startRef || !endRef) return false;

    dtPolyRef polys[256];
    int polyCount = 0;

    navQuery->findPath(startRef, endRef, nearestStart, nearestEnd,
                    &filter, polys, &polyCount, 256);

    if (polyCount <= 0) return false;

    // Straight path
    float straightPath[256 * 3];
    unsigned char straightPathFlags[256];
    dtPolyRef straightPathPolys[256];
    int straightPathCount = 0;

    navQuery->findStraightPath(nearestStart, nearestEnd,
                            polys, polyCount,
                            straightPath,
                            straightPathFlags,
                            straightPathPolys,
                            &straightPathCount,
                            256);

    outPath.clear();
    for (int i = 0; i < straightPathCount; ++i)
    {
        outPath.push_back(straightPath[3*i + 0]);
        outPath.push_back(straightPath[3*i + 1]);
        outPath.push_back(straightPath[3*i + 2]);
    }

    return !outPath.empty();
}

bool Level::SampleRandomPoint(float* outPt)
{
    dtQueryFilter filter;
    filter.setIncludeFlags(0x01);
    filter.setExcludeFlags(0);
    if (!navQuery) return false;

    dtPolyRef ref = 0;
    dtStatus status = navQuery->findRandomPoint(&filter, frand, &ref, outPt);
    if (dtStatusFailed(status) || !ref)
        return false;

    return true;
}

void Level::saveContent(fs::path contentFile, std::ostream &os)
{
    // This is an example lvl.json file
    // {
    //     renderables: {
    //         instanceId: {
    //             assetId: 'guid of renderable file that is saved to disk',
    //             transform: {
    //                 t: [], r: [], s:[]
    //             }
    //         }
    //     },
    //     ais: {
    //         instanceId: {
    //             assetId: "guid of the ai file that is saved to disk",
    //         }
    //     }
    // }

    bs::ptree contentJSON;

    bs::ptree renderablesNode; // array node for all renderables
    bs::ptree aisNode; // array node for all renderables

    for (size_t i = 0; i < renderables.size(); i++)
    {
        auto r = renderables.at(i);
        if (std::shared_ptr<Serializable> serializable =std::dynamic_pointer_cast<Serializable>(r))
        {
            bs::ptree renderableNode;

            // ID
            renderableNode.put("assetId", r->GetGuid());

            // Transform object
            bs::ptree transformNode;

            // Translation
            bs::ptree tNode;
            auto t = r->GetPosition();
            tNode.push_back(std::make_pair("", bs::ptree(std::to_string(t.x))));
            tNode.push_back(std::make_pair("", bs::ptree(std::to_string(t.y))));
            tNode.push_back(std::make_pair("", bs::ptree(std::to_string(t.z))));
            transformNode.add_child("t", tNode);

            // Rotation
            bs::ptree rNode;
            auto rot = r->GetRot();
            rNode.push_back(std::make_pair("", bs::ptree(std::to_string(rot.x))));
            rNode.push_back(std::make_pair("", bs::ptree(std::to_string(rot.y))));
            rNode.push_back(std::make_pair("", bs::ptree(std::to_string(rot.z))));
            rNode.push_back(std::make_pair("", bs::ptree(std::to_string(rot.w))));
            transformNode.add_child("r", rNode);

            // Scale
            bs::ptree sNode;
            auto s = r->GetScale();
            sNode.push_back(std::make_pair("", bs::ptree(std::to_string(s.x))));
            sNode.push_back(std::make_pair("", bs::ptree(std::to_string(s.y))));
            sNode.push_back(std::make_pair("", bs::ptree(std::to_string(s.z))));
            transformNode.add_child("s", sNode);

            renderableNode.add_child("transform", transformNode);

            if (auto character = std::dynamic_pointer_cast<Character>(r))
            {

                if (auto aiController = std::dynamic_pointer_cast<AI::AI>(character->controller))
                {
                    bs::ptree variablesNode;
                    for (const auto& vec : aiController->getVariables())
                    {
                        bs::ptree vNode;
                        vNode.put("x", vec.x);
                        vNode.put("y", vec.y);
                        vNode.put("z", vec.z);

                        variablesNode.push_back(std::make_pair("", vNode));
                    }

                    renderableNode.add_child("patrol_variables", variablesNode);
                }

            }

            // Append to array
            renderablesNode.add_child(serializable->getInstanceId(), renderableNode);
        }
    }

    for (size_t i = 0; i < AIs.size(); i++)
    {
        auto ai = AIs[i];
        if (auto serializable = std::dynamic_pointer_cast<Serializable>(ai))
        {
            bs::ptree aiNode;

            // ID
            aiNode.put("assetId", serializable->getAssetId());
            // aiNode.put("renderableInstanceId", ai->GetCharacterGuid()); Lets serialize this in the AI file.
            aisNode.add_child(serializable->getInstanceId(), aiNode);
        }
    }

    // Add to root object
    contentJSON.add_child("renderables", renderablesNode);
    contentJSON.add_child("ais", aisNode);

    bs::write_json(contentFile.string(), contentJSON);
}