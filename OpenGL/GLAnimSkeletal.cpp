#include "GLAnimSkeletal.h"

#include "../Models/lib3dx/xUtils.h"
#include "../Models/lib3dx/xSkeleton.h"

void GLAnimSkeletal::Initialize()
{
    m_GPUprogram = m_SkeletalShader.Initialize();
    if (m_GPUprogram)
    {
        uBones        = glGetUniformLocationARB(m_GPUprogram, "bones");
        aBoneIdxWghts = glGetAttribLocationARB (m_GPUprogram, "boneIdxWghts");
    }
}
void GLAnimSkeletal::ReinitializeGL()
{
    if (m_GPUprogram)
    {
        m_SkeletalShader.Invalidate();
        Initialize();
    }
}

void GLAnimSkeletal::BeginAnimation()
{
    m_notForceCPU = !m_forceCPU && g_UseCustomShader;
    if (m_GPUprogram && m_notForceCPU)
        m_prevShader = m_SkeletalShader.Start();
    sft_vertices = NULL;
    sft_normals  = NULL;
}
void GLAnimSkeletal::EndAnimation()
{
    if (m_GPUprogram && m_notForceCPU)
    {
        glDisableVertexAttribArrayARB(aBoneIdxWghts);
        if (m_prevShader)
            m_prevShader->Start();
        else
            m_SkeletalShader.Suspend();
    }
    if (sft_vertices)
    {
        delete[] sft_vertices;
        sft_vertices = NULL;
    }
    if (sft_normals)
    {
        delete[] sft_normals;
        sft_normals = NULL;
    }
}

void GLAnimSkeletal::SetBones(GLsizei noOfBones, const xMatrix *bonesM, const xVector4 *bonesQ)
{
    if (m_GPUprogram && m_notForceCPU) {
        glUniform4fvARB(uBones, noOfBones*2, (GLfloat *)bonesQ);
    }
    else {
        sft_bonesM    = bonesM;
        sft_bonesQ    = bonesQ;
        sft_boneCount = noOfBones;
    }
}

void GLAnimSkeletal::SetElement(const xElement *element, bool VBO)
{
    if (m_GPUprogram && m_notForceCPU) {
        xDWORD stride = element->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        glEnableVertexAttribArrayARB(aBoneIdxWghts);
        if (VBO) {
            glVertexAttribPointerARB (aBoneIdxWghts, 4, GL_FLOAT, GL_FALSE, stride, (void *)(3*sizeof(xFLOAT)));
            glBindBufferARB ( GL_ARRAY_BUFFER_ARB, element->renderData.vertexB );
            glVertexPointer (3, GL_FLOAT, stride, 0);

            /************************* LOAD NORMALS ****************************/
            if (!g_SelectionRendering && element->renderData.normalP) {
                glBindBufferARB ( GL_ARRAY_BUFFER_ARB, element->renderData.normalB );
                glNormalPointer ( GL_FLOAT, sizeof(xVector3), 0 );
                glEnableClientState(GL_NORMAL_ARRAY);
            }
        }
        else {
            glVertexAttribPointerARB    (aBoneIdxWghts, 4, GL_FLOAT, GL_FALSE,
                stride, element->renderData.verticesSP->bone);
            glVertexPointer (3, GL_FLOAT, stride, element->renderData.verticesP);

            /************************* LOAD NORMALS ****************************/
            if (!g_SelectionRendering && element->renderData.normalP) {
                glNormalPointer ( GL_FLOAT, sizeof(xVector3), element->renderData.normalP );
                glEnableClientState(GL_NORMAL_ARRAY);
            }
        }
    }
    else {
        xSkinnedData sData = xElement_GetSkinnedElement(element, sft_bonesM);
        sft_vertices = sData.verticesP;
        sft_normals  = sData.normalsP;
        if (VBO) glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
        glVertexPointer (3, GL_FLOAT, sizeof(xVector3), sft_vertices);
        glNormalPointer (GL_FLOAT, sizeof(xVector3), sft_normals);
        glEnableClientState(GL_NORMAL_ARRAY);
    }
}

/**************** generic implementation ******************/

void GLAnimSkeletal::SetBoneIdxWghts(GLsizei stride, const GLfloat *pointer)
{
    if (m_GPUprogram && m_notForceCPU) {
        glEnableVertexAttribArrayARB(aBoneIdxWghts);
        glVertexAttribPointerARB(aBoneIdxWghts, 4, GL_FLOAT, GL_FALSE, stride, pointer);
    }
    else {
        sft_boneIdxWghts  = pointer;
        sft_boneIdxStride = stride;
        sft_boneIdxArray  = true;
    }
}

void GLAnimSkeletal::SetVertices(GLsizei stride, const GLfloat *pointer, int count)
{
    if (m_GPUprogram && m_notForceCPU)
        glVertexPointer   (3, GL_FLOAT, stride, pointer);
    else {
        xBYTE    *src = (xBYTE *) pointer;
        xVector3 *dst = new xVector3[count];
        xVector3 *itr = dst;

        if (sft_boneIdxArray) {
            const GLfloat *wghts = sft_boneIdxWghts;
            GLsizei boneStride = sft_boneIdxStride - 4*sizeof(GLfloat);

            memset(dst, 0, count*sizeof(xVector4));
            for (int i = 0; i<count; ++i, ++itr, src += stride)
            {
                xVector4 vec; vec.init(*((xVector3 *)src), 1.f);
                for (int j=0; j<4; ++j, ++wghts)
                    (*itr) += (sft_bonesM[(int)floor(*wghts)]
                              * vec * 10*(*wghts - floor(*wghts))).vector3;
                wghts = (GLfloat *)((xBYTE *) wghts + boneStride);
            }
        }
        else {
            int   i1 = (int) floor(sft_boneIdxWghts[0]);
            float w1 = (sft_boneIdxWghts[0] - i1)*10;
            int   i2 = (int) floor(sft_boneIdxWghts[1]);
            float w2 = (sft_boneIdxWghts[1] - i2)*10;
            int   i3 = (int) floor(sft_boneIdxWghts[2]);
            float w3 = (sft_boneIdxWghts[2] - i3)*10;
            int   i4 = (int) floor(sft_boneIdxWghts[3]);
            float w4 = (sft_boneIdxWghts[3] - i4)*10;

            for (int i = 0; i<count; ++i, ++itr, src += stride)
            {
                xVector4 vec; vec.init(*((xVector3 *)src), 1.f);
                *itr  = (sft_bonesM[i1] * vec * w1).vector3;
                *itr += (sft_bonesM[i2] * vec * w2).vector3;
                *itr += (sft_bonesM[i3] * vec * w3).vector3;
                *itr += (sft_bonesM[i4] * vec * w4).vector3;
            }
        }

        glVertexPointer (3, GL_FLOAT, sizeof(xVector3), dst);
        sft_vertices = dst;
    }
}

void GLAnimSkeletal::SetBoneIdxWghts(const GLfloat pointer[4])
{
    if (m_GPUprogram && m_notForceCPU)
        glVertexAttrib4fv(aBoneIdxWghts, pointer);
    else
    {
        sft_boneIdxWghts  = pointer;
        sft_boneIdxStride = 0;
        sft_boneIdxArray  = false;
    }
}

void GLAnimSkeletal::SetVertex(const GLfloat vertex[3])
{
    if (m_GPUprogram && m_notForceCPU)
        glVertex3fv(vertex);
    else
    {
        int   i1 = (int) floor(sft_boneIdxWghts[0]);
        float w1 = (sft_boneIdxWghts[0] - i1)*10;
        int   i2 = (int) floor(sft_boneIdxWghts[1]);
        float w2 = (sft_boneIdxWghts[1] - i2)*10;
        int   i3 = (int) floor(sft_boneIdxWghts[2]);
        float w3 = (sft_boneIdxWghts[2] - i3)*10;
        int   i4 = (int) floor(sft_boneIdxWghts[3]);
        float w4 = (sft_boneIdxWghts[3] - i4)*10;

        xVector4 vec; vec.init(*((xVector3 *)vertex), 1.f);
        xVector4 res = sft_bonesM[i1] * vec * w1;
        res += sft_bonesM[i2] * vec * w2;
        res += sft_bonesM[i3] * vec * w3;
        res += sft_bonesM[i4] * vec * w4;

        glVertex3fv(res.vector3.xyz);
    }
}
