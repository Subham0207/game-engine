#include  <Level/Level.hpp>
#include <EngineState.hpp>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <AI/NavMeshBuilder.hpp>
#include <Modals/3DModelType.hpp>
namespace bs = boost::property_tree;
namespace fs = std::filesystem;

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
        for (const auto &renderableNode : levelContent.get_child("renderables")) {
            const bs::ptree &renderable = renderableNode.second;

            // Read the id
            std::string id = renderable.get<std::string>("id");

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

            if(extension ==  ".model")
            {
                auto model = new Model();
                model->load(contentFilePath.parent_path(), id);
                model->setModelMatrix(M);
                auto filename = contentFilePath.stem().filename().string();
                model->setFileName(filename);
                this->renderables->push_back(model);
            }
            if(extension ==  ".character")
            {
                auto character = new Character();
                character->load(contentFilePath.parent_path(), id);
                character->setModelMatrix(M);
                this->renderables->push_back(character);
            }


        }

    } catch (const bs::ptree_error &e) {
        std::cerr << "Error reading JSON: " << e.what() << "\n";
    }
}

void Level::setupLevelVertices(std::vector<float> navVerts, std::vector<unsigned int> navTris)
{
    lvlVerticesShader = new Shader("./Shaders/debug/debug.vert","./Shaders/debug/debug.frag");
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

void Level::BuildLevelNavMesh()
{
    int baseVert = 0;

    for(int i=0;i<renderables->size();i++)
    {
        auto r = renderables->at(i);

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
    NavMeshBuilder builder;

    setupLevelVertices(verts, tris); // Here we get vertices from all models in the level and convert into 1 mesh. And that mesh is used to build th nav mesh.
    std::vector<int> i_vec(tris.begin(), tris.end());
    bool ok = builder.build(verts.data(), (int)verts.size()/3,
                            i_vec.data(), (int)tris.size()/3,
                            cfg);

    if (ok)
    {
        navMesh = builder.getNavMesh();
        navQuery = builder.getNavMeshQuery();

        debugNavMeshShader = new Shader("./Shaders/debug/debug.vert","./Shaders/debug/debug.frag");
        debugNavMeshShader->use();
        std::vector<float> outVerts;
        std::vector<unsigned> outTris;
        builder.getDebugNavmesh(
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
    //{
    //     "renderables":
    //     [
    //         {
    //             "id": "guid of the renderable",
    //             "transform": {
    //                 "t": [0, 0, 0],
    //                 "r": [0, 0, 0],
    //                 "s": [0, 0, 0]
    //             }
    //         }
    //     ]
    // }

    bs::ptree contentJSON;

    bs::ptree renderablesArray; // array node for all renderables

    for (size_t i = 0; i < renderables->size(); i++)
    {
        auto& r = renderables->at(i);

        bs::ptree renderableNode;

        // ID
        renderableNode.put("id", r->GetGuid());

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

        // Append to array
        renderablesArray.push_back(std::make_pair("", renderableNode));
    }

    // Add to root object
    contentJSON.add_child("renderables", renderablesArray);

    bs::write_json(contentFile.string(), contentJSON);
}