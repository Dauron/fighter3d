#ifndef __incl_GLAnimSkeletal_h
#define __incl_GLAnimSkeletal_h     1

#include "../../Utils/Singleton.h"
#include "GLShader.h"
#include "../../Models/lib3dx/xModel.h"

#define g_AnimSkeletal GLAnimSkeletal::GetSingleton()

class GLAnimSkeletal : public Singleton<GLAnimSkeletal>
{
    bool      m_forceCPU;    // should skeletizing be done by CPU, even if GPU is aviable?
    bool      m_notForceCPU; // inverse of above - value is valid only between BeginAnimation() and EndAnimation()

    // temporary storage used by CPU algorithm between BeginAnimation() and EndAnimation()
    const xMatrix     *sft_bonesM;
    const xQuaternion *sft_bonesQ;
    const xPoint3     *sft_bonesR;
    const xPoint3     *sft_bonesT;
    GLsizei            sft_boneCount;
    const GLfloat     *sft_boneIdxWghts;
    GLsizei            sft_boneIdxStride;
    bool               sft_boneIdxArray;
    const xVector3    *sft_vertices;
    const xVector3    *sft_normals;

    ShaderSkeletal *currSkeletalShader;

public:
    GLAnimSkeletal() : m_forceCPU(false) {}

    bool HardwareEnabled()          { return currSkeletalShader; }
    void ForceSoftware(bool enable) { m_forceCPU = enable; }

    void BeginAnimation();
    void EndAnimation();

    void SetBones(GLsizei noOfBones,
                              const xMatrix *bonesM,
                              const xQuaternion *bonesQ,
                              const xPoint3     *bonesR,
                              const xPoint3     *bonesT,
                              const xElement *element, bool VBO);
    void SetElement(const xElement *element, const xElementInstance *instance, bool VBO = false);

    /**************** generic implementation ******************/

    void SetBoneIdxWghts(GLsizei stride, const GLfloat *pointer);
    void SetVertices(GLsizei stride, const GLfloat *pointer, int count);

    void SetBoneIdxWghts(const GLfloat pointer[4]);
    void SetVertex(const GLfloat vertex[3]);
};

#endif
