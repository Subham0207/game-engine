#include <list>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h> 

class Technique
{
public:

    Technique();

    virtual ~Technique();

    virtual bool Init();

    void Enable();

    unsigned int GetProgram() const { return m_shaderProg; }

protected:

    bool AddShader(enum ShaderType, const char* pFilename);

    bool Finalize();

    int GetUniformLocation(const char* pUniformName);

    unsigned int m_shaderProg = 0;

private:

    typedef std::list<unsigned int> ShaderObjList;
    ShaderObjList m_shaderObjList;
};

class IRenderCallbacks
{
public:
    virtual void DrawStartCB(unsigned int DrawIndex) = 0;
};

class PickingTechnique : public Technique, public IRenderCallbacks
{
public:

    PickingTechnique();

    virtual bool Init();

    void SetWVP(const glm::mat4& WVP);

    void SetObjectIndex(unsigned int ObjectIndex);
    
    void DrawStartCB(unsigned int DrawIndex);
    
private:
    
    unsigned int m_WVPLocation;
    unsigned int m_drawIndexLocation;
    unsigned int m_objectIndexLocation;
};

class PickingTexture
{
public:
    PickingTexture();

    ~PickingTexture();

    bool Init(unsigned int WindowWidth, unsigned int WindowHeight);

    void EnableWriting();
    
    void DisableWriting();
    
    struct PixelInfo {
        float ObjectID;
        float DrawID;
        float PrimID;
        
        PixelInfo()
        {
            ObjectID = 0.0f;
            DrawID = 0.0f;
            PrimID = 0.0f;
        }
    };

    PixelInfo ReadPixel(unsigned int x, unsigned int y);
    
private:
    unsigned int m_fbo;
    unsigned int m_pickingTexture;
    unsigned int m_depthTexture;
};