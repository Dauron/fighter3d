#include "GLAnimSkeletal.h"

#include "Extensions/ARB_vertex_buffer_object.h"
#include "../Models/lib3dx/xSkeleton.h"

int aBoneIdxWghts = -1;

void GLAnimSkeletal::BeginAnimation()
{
    currSkeletalShader = (ShaderSkeletal*)GLShader::currShader;

    if (GLShader::SkeletonState() != xState_On)
        currSkeletalShader = 0;
    aBoneIdxWghts = -1;

    m_notForceCPU = !m_forceCPU && Config::EnableShaders && currSkeletalShader && currSkeletalShader->currProgram;
    sft_vertices  = NULL;
    sft_normals   = NULL;
}

void GLAnimSkeletal::EndAnimation()
{
    if (m_notForceCPU && currSkeletalShader->currProgram)
    {
        if (aBoneIdxWghts > -1)
            glDisableVertexAttribArrayARB(aBoneIdxWghts);
        //GLShader::Suspend();
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

void GLAnimSkeletal::SetBones(GLsizei noOfBones, const xMatrix *bonesM, const xVector4 *bonesQ,
                              const xElement *element, bool VBO)
{
    currSkeletalShader = (ShaderSkeletal*)GLShader::currShader;
    if (GLShader::SkeletonState() != xState_On)
        currSkeletalShader = 0;
    m_notForceCPU = !m_forceCPU && Config::EnableShaders && currSkeletalShader && currSkeletalShader->currProgram;

    if (m_notForceCPU) {
        glUniform4fvARB(currSkeletalShader->uBones, noOfBones*2, (GLfloat *)bonesQ);

        if (element)
        {
            glEnableVertexAttribArrayARB(currSkeletalShader->aBoneIdxWghts);
            aBoneIdxWghts = currSkeletalShader->aBoneIdxWghts;
            xDWORD stride = element->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
            if (VBO)
                glVertexAttribPointerARB (currSkeletalShader->aBoneIdxWghts, 4, GL_FLOAT, GL_FALSE, stride, (void *)(3*sizeof(xFLOAT)));
            else
                glVertexAttribPointerARB (currSkeletalShader->aBoneIdxWghts, 4, GL_FLOAT, GL_FALSE,
                    stride, element->renderData.verticesSP->bone);
        }
    }
    else {
        sft_bonesM    = bonesM;
        sft_bonesQ    = bonesQ;
        sft_boneCount = noOfBones;
    }
}

void GLAnimSkeletal::SetElement(const xElement *element, const xElementInstance *instance, bool VBO)
{
    if (m_notForceCPU) {
        xDWORD stride = element->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        if (VBO) {
            glBindBufferARB ( GL_ARRAY_BUFFER_ARB, element->renderData.gpuMain.vertexB );
            glVertexPointer (3, GL_FLOAT, stride, 0);
            if (State::RenderingShadows) glTexCoordPointer (3, GL_FLOAT, stride, 0);

            /************************* LOAD NORMALS ****************************/
            if ((GLShader::NeedNormals()) && element->renderData.gpuMain.normalB) {
                glBindBufferARB ( GL_ARRAY_BUFFER_ARB, element->renderData.gpuMain.normalB );
                glNormalPointer ( GL_FLOAT, sizeof(xVector3), 0 );
                glEnableClientState(GL_NORMAL_ARRAY);
                glBindBufferARB ( GL_ARRAY_BUFFER_ARB, element->renderData.gpuMain.vertexB );
            }
        }
        else {
            glVertexPointer (3, GL_FLOAT, stride, element->renderData.verticesP);
            if (State::RenderingShadows) glTexCoordPointer (3, GL_FLOAT, stride, element->renderData.verticesP);

            /************************* LOAD NORMALS ****************************/
            if (element->renderData.normalP) {
                glNormalPointer ( GL_FLOAT, sizeof(xVector3), element->renderData.normalP );
                glEnableClientState(GL_NORMAL_ARRAY);
            }
        }
    }
    else {
        xSkinnedData sData = element->GetSkinnedVertices(sft_bonesM);
        sft_vertices = sData.verticesP;
        sft_normals  = sData.normalsP;
        if (VBO) glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
        glVertexPointer (3, GL_FLOAT, sizeof(xVector3), sft_vertices);
        if (State::RenderingShadows) glTexCoordPointer (3, GL_FLOAT, sizeof(xVector3), sft_vertices);
        if ((VBO && GLShader::NeedNormals()) ||
            (!VBO && !State::RenderingSelection))
        {
            glNormalPointer (GL_FLOAT, sizeof(xVector3), sft_normals);
            glEnableClientState(GL_NORMAL_ARRAY);
        }
    }
}

/**************** generic implementation ******************/

void GLAnimSkeletal::SetBoneIdxWghts(GLsizei stride, const GLfloat *pointer)
{
    if (m_notForceCPU) {
        glEnableVertexAttribArrayARB(currSkeletalShader->aBoneIdxWghts);
        glVertexAttribPointerARB(currSkeletalShader->aBoneIdxWghts, 4, GL_FLOAT, GL_FALSE, stride, pointer);
    }
    else {
        sft_boneIdxWghts  = pointer;
        sft_boneIdxStride = stride;
        sft_boneIdxArray  = true;
    }
}

void GLAnimSkeletal::SetVertices(GLsizei stride, const GLfloat *pointer, int count)
{
    if (m_notForceCPU)
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
    if (m_notForceCPU)
        glVertexAttrib4fv(currSkeletalShader->aBoneIdxWghts, pointer);
    else
    {
        sft_boneIdxWghts  = pointer;
        sft_boneIdxStride = 0;
        sft_boneIdxArray  = false;
    }
}

void GLAnimSkeletal::SetVertex(const GLfloat vertex[3])
{
    if (m_notForceCPU)
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
