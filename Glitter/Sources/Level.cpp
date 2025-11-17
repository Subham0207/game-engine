#include  <Level/Level.hpp>
#include <EngineState.hpp>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
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

void Level::saveContent(fs::path contentFile, std::ostream& os)
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