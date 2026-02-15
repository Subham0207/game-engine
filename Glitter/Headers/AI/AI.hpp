#include <glm/glm.hpp>
#include <Serializable.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>

#include "Controls/Controller.hpp"

namespace Controls{
    class PlayerController;
}
class Character;
namespace AI
{
    class AI: public Serializable, public Controls::Controller
    {
    public:
        AI()=default;
        std::string GetClassId() const override { return "AI"; }
        AI(std::shared_ptr<Character> character, std::string filename);
        void OnStart() override;
        void OnTick(float deltaTime) override;
        void OnDestroy() override{};
        void setPath(std::vector<glm::vec3> path);
        void calculatePath(glm::vec3 startingPos, glm::vec3 targetPos);
        // std::shared_ptr<Controls::PlayerController> playerController;
        
        void setFilename(std::string filename){this->filename = filename;}

        std::string getCharacterInstanceId(){return controlledCharacterInstanceId;}
        void setCharacterInstanceId(std::string controlledCharacterInstanceId){this->controlledCharacterInstanceId=controlledCharacterInstanceId;}
        virtual const std::string contentName() override {return filename; }

    protected:
        virtual const std::string typeName() const override {return "AI"; }

        virtual void saveContent(fs::path contentFileLocation, std::ostream& os) override;
        virtual void loadContent(fs::path contentFileLocation, std::istream& is) override;
    private:

        glm::vec3 targetDirection;
        bool targetDirChoosen;

        float elapsedTime;

        float arrivalRadius;
        std::vector<glm::vec3> path;
        int currentPathIndex;

        std::string filename;

        std::string controlledCharacterInstanceId;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & filename;
            ar & controlledCharacterInstanceId;
            ar & arrivalRadius;
        }
    };
}