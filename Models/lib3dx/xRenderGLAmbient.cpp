#include "xRenderGL.h"
#include "../../OpenGL/GLAnimSkeletal.h"

/********************************* ambient model ************************************/
void RenderElementAmbientLST(bool transparent, const xFieldOfView &FOV, const xLightVector &lights,
                    xElement *elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderElementAmbientLST(transparent, FOV, lights, selem, modelInstance);

    if (!elem->renderData.verticesC
        || (transparent && !elem->transparent)
        || (!transparent && !elem->opaque)) return;

    xElementInstance &instance = modelInstance.elementInstanceP[elem->id];
    xMatrix mtxTrasformation = elem->matrix * modelInstance.location;
    xVector3 center = mtxTrasformation.preTransformP(instance.bsCenter);
    if (!FOV.CheckSphere(center, instance.bsRadius) ||
        !FOV.CheckBox(instance.bbBox.TransformatedPoints(mtxTrasformation)) )
    {
        ++Performance.CulledElements;
        return;
    }

    int lightsC = 0;
    xColor ambient; ambient.zeroQ();
    xLightVector::const_iterator iterL = lights.begin(), endL = lights.end();
    for (; iterL != endL; ++iterL)
        if (iterL->elementReceivesLight(center, instance.bsRadius))
        {
            ambient.vector3 += iterL->ambient.vector3;
            ++lightsC;
        }
    if (!lightsC) return;
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient.xyzw);

    /************************** PREPARE LST  ****************************/
    xDWORD &listID    = transparent ? elem->renderData.gpuMain.listIDTransp : elem->renderData.gpuMain.listID;
    xDWORD &listIDTex = transparent ? elem->renderData.gpuMain.listIDTexTransp : elem->renderData.gpuMain.listIDTex;
    bool textured = false;

    if (elem->skeletized)
        GLShader::EnableSkeleton(xState_On);

    if (!listID)
    {
        elem->renderData.mode = xGPURender::LIST;
        glNewList(listID = glGenLists(1), GL_COMPILE);

        if (elem->skeletized) {
            g_AnimSkeletal.BeginAnimation();
            g_AnimSkeletal.SetBones (modelInstance.bonesC, modelInstance.bonesM, modelInstance.bonesQ, elem, false);
            g_AnimSkeletal.SetElement(elem, &instance);
        }
        else
        {
            if (elem->textured)
                glVertexPointer   (3, GL_FLOAT, sizeof(xVertexTex), elem->renderData.verticesTP);
            else
                glVertexPointer   (3, GL_FLOAT, sizeof(xVertex), elem->renderData.verticesP);
            /////////////////////////// LOAD NORMALS ///////////////////////////
            if (elem->renderData.normalP)
            {
                glNormalPointer (GL_FLOAT, sizeof(xVector3), elem->renderData.normalP);
                glEnableClientState(GL_NORMAL_ARRAY);
            }
        }

        xFaceList *faceL = elem->faceListP;
        xMaterial *m_currentMaterial = (xMaterial*)1;
        for(int i=elem->faceListC; i; --i, ++faceL)
        {
            if ((transparent && (!faceL->materialP || faceL->materialP->transparency == 0.f)) ||
                (!transparent && faceL->materialP && faceL->materialP->transparency > 0.f) )
                continue;
            if (elem->textured && faceL->materialP && faceL->materialP->texture.htex)
            {
                textured = true;
                continue;
            }
            if (faceL->materialP != m_currentMaterial)
                xRenderGL::SetMaterial(elem->color, m_currentMaterial = faceL->materialP, false);
            glDrawElements(GL_TRIANGLES, 3*faceL->indexCount, GL_UNSIGNED_SHORT, elem->renderData.facesP+faceL->indexOffset);
        }
        if (!textured && elem->renderData.normalP) glDisableClientState(GL_NORMAL_ARRAY);
        if (!textured && elem->skeletized)
            g_AnimSkeletal.EndAnimation();

        glEndList();
    }

    if (textured && elem->textured && !listIDTex)
    {
        glNewList(listIDTex = glGenLists(1), GL_COMPILE);

        if (elem->skeletized) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer (2, GL_FLOAT, sizeof(xVertexTexSkel), &(elem->renderData.verticesTSP->tx));
            g_AnimSkeletal.SetBones (modelInstance.bonesC, modelInstance.bonesM, modelInstance.bonesQ, elem, false);
        }
        else {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer (2, GL_FLOAT, sizeof(xVertexTex), &(elem->renderData.verticesTP->tx));
        }

        xFaceList *faceL = elem->faceListP;
        xMaterial *m_currentMaterial = (xMaterial*)1;
        for(int i=elem->faceListC; i; --i, ++faceL)
        {
            if ((transparent && (!faceL->materialP || faceL->materialP->transparency == 0.f)) ||
                (!transparent && faceL->materialP && faceL->materialP->transparency > 0.f) )
                continue;
            if (!faceL->materialP || !faceL->materialP->texture.htex)
                continue;
            if (faceL->materialP != m_currentMaterial)
                xRenderGL::SetMaterial(elem->color, m_currentMaterial = faceL->materialP, false);
            glDrawElements(GL_TRIANGLES, 3*faceL->indexCount, GL_UNSIGNED_SHORT, elem->renderData.facesP+faceL->indexOffset);
        }
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if (elem->renderData.normalP) glDisableClientState(GL_NORMAL_ARRAY);
        if (elem->skeletized)
            g_AnimSkeletal.EndAnimation();

        glEndList();
    }

    /************************* USE LST ****************************/
    if (listID)
    {
        glPushMatrix();
        {
            glMultMatrixf(&elem->matrix.x0);
            GLShader::EnableTexturing(xState_Off);
            GLShader::Start();
            glCallList(listID);
            if (listIDTex)
            {
                GLShader::EnableTexturing(xState_On);
                GLShader::Start();
                glCallList(listIDTex);
            }
        }
        glPopMatrix();
    }
    GLShader::EnableSkeleton(xState_Off);
}
void RenderElementAmbientVBO(bool transparent, const xFieldOfView &FOV, const xLightVector &lights,
                    xElement *elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderElementAmbientVBO(transparent, FOV, lights, selem, modelInstance);

    if (!elem->renderData.verticesC
        || (transparent && !elem->transparent)
        || (!transparent && !elem->opaque)) return;

    xElementInstance &instance = modelInstance.elementInstanceP[elem->id];
    xMatrix mtxTrasformation = elem->matrix * modelInstance.location;
    xVector3 center = mtxTrasformation.preTransformP(instance.bsCenter);
    if (!FOV.CheckSphere(center, instance.bsRadius) ||
        !FOV.CheckBox(instance.bbBox.TransformatedPoints(mtxTrasformation)) )
    {
        ++Performance.CulledElements;
        return;
    }

    int lightsC = 0;
    xColor ambient; ambient.zeroQ();
    xLightVector::const_iterator iterL = lights.begin(), endL = lights.end();
    for (; iterL != endL; ++iterL)
        if (iterL->elementReceivesLight(center, instance.bsRadius))
        {
            ambient.vector3 += iterL->ambient.vector3;
            ++lightsC;
        }
    if (!lightsC) return;
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient.xyzw);

    /************************* INIT VBO ****************************/
    if (elem->renderData.mode == xGPURender::NONE)
        xRenderGL::InitVBO(elem);

    /************************* LOAD VERTICES ****************************/
    glPushMatrix();
    glMultMatrixf(&elem->matrix.matrix[0][0]);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
    if (elem->skeletized) {
        if (elem->textured) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer (2, GL_FLOAT, sizeof(xVertexTexSkel), (void *)(7*sizeof(xFLOAT)));
        }
        GLShader::EnableSkeleton(xState_On);
        GLShader::Start();
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones  (modelInstance.bonesC, modelInstance.bonesM, modelInstance.bonesQ, elem, true);
        g_AnimSkeletal.SetElement(elem, &instance, true);
    }
    else
    {
        if (elem->textured) {
            glVertexPointer   (3, GL_FLOAT, sizeof(xVertexTex), 0);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer (2, GL_FLOAT, sizeof(xVertexTex), (void *)(3*sizeof(xFLOAT)));
        }
        else
            glVertexPointer   (3, GL_FLOAT, sizeof(xVertex), 0);
    }

    /************************* RENDER FACES ****************************/
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.indexB );
    xMaterial *m_currentMaterial = (xMaterial*)1;
    xFaceList *faceL = elem->faceListP;
    for(int i=elem->faceListC; i; --i, ++faceL)
    {
        if ((transparent && (!faceL->materialP || faceL->materialP->transparency == 0.f)) ||
            (!transparent && faceL->materialP && faceL->materialP->transparency > 0.f) )
            continue;
        if (faceL->materialP != m_currentMaterial)
        {
            xRenderGL::SetMaterial(elem->color, m_currentMaterial = faceL->materialP);
            if (elem->skeletized) g_AnimSkeletal.SetBones  (modelInstance.bonesC, modelInstance.bonesM, modelInstance.bonesQ, elem, true);
        }
        glDrawElements(GL_TRIANGLES, 3*faceL->indexCount, GL_UNSIGNED_SHORT, (void*)(faceL->indexOffset*3*sizeof(xWORD)));
    }
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB ( GL_ARRAY_BUFFER_ARB, 0 );

    if (elem->textured)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    if (elem->skeletized)
        g_AnimSkeletal.EndAnimation();
    GLShader::EnableSkeleton(xState_Off);

    glPopMatrix();
}

void xRenderGL :: RenderAmbient(xModel &model, xModelInstance &instance, const xLightVector &lights,
                              bool transparent, const xFieldOfView &FOV)
{
    if ((transparent  && !model.transparent) ||
        (!transparent && !model.opaque)) return;

    InitTextures(model);
    glEnableClientState(GL_VERTEX_ARRAY);

    glPushMatrix();
    glMultMatrixf(&instance.location.x0);

    for (xElement *elem = model.kidsP; elem; elem = elem->nextP)
        if (UseVBO)
            RenderElementAmbientVBO(transparent, FOV, lights, elem, instance);
        else
            RenderElementAmbientLST(transparent, FOV, lights, elem, instance);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
}
