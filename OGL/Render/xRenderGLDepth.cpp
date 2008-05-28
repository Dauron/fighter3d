#include "xRenderGL.h"
#include "../GLAnimSkeletal.h"

/********************************* vertices && element selections ************************************/
void RenderElementDepthLST(
                            xElement * elem, xModelInstance &modelInstance,
                            bool transparent, const xFieldOfView &FOV )
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderElementDepthLST( selem, modelInstance, transparent, FOV );

    if (!elem->renderData.verticesC
        || (transparent && !elem->transparent)
        || (!transparent && !elem->opaque)) return;

    xElementInstance &instance = modelInstance.elementInstanceP[elem->id];
    xMatrix mtxTrasformation = elem->matrix * modelInstance.location;
    if (!FOV.CheckSphere(mtxTrasformation.preTransformP(instance.bsCenter), instance.bsRadius) ||
        !FOV.CheckBox(instance.bbBox.TransformatedPoints(mtxTrasformation)) )
    {
        ++Performance.CulledElements;
        return;
    }

    glPushMatrix();
    glMultMatrixf(&elem->matrix.x0);

    if (elem->skeletized) {
        GLShader::EnableSkeleton(xState_On);
        GLShader::Start();
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones  (modelInstance.bonesC, modelInstance.bonesM, modelInstance.bonesQ, elem, false);
        g_AnimSkeletal.SetElement(elem, &instance, false);
    }
    else
        glVertexPointer   (3, GL_FLOAT, (elem->textured) ? sizeof(xVertexTex) : sizeof(xVertex), elem->renderData.verticesP);

    /************************* RENDER FACES ****************************/
    if ((transparent && !elem->opaque) ||
        (!transparent && !elem->transparent))
        glDrawElements (GL_TRIANGLES, 3*elem->facesC, GL_UNSIGNED_SHORT, elem->renderData.facesP);
    else
    {
        xFaceList *faceL = elem->faceListP;
        for(int i=elem->faceListC; i; --i, ++faceL)
        {
            if ((transparent && (!faceL->materialP || faceL->materialP->transparency == 0.f)) ||
                (!transparent && faceL->materialP && faceL->materialP->transparency > 0.f) )
                continue;
            glDrawElements(GL_TRIANGLES, 3*faceL->indexCount, GL_UNSIGNED_SHORT, elem->renderData.facesP+faceL->indexOffset);
        }
    }

    if (elem->skeletized)
        g_AnimSkeletal.EndAnimation();
    GLShader::EnableSkeleton(xState_Off);

    glPopMatrix();
}

void RenderElementDepthVBO( xElement * elem, xModelInstance &modelInstance,
                            bool transparent, const xFieldOfView &FOV )
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderElementDepthVBO( selem, modelInstance, transparent, FOV );

    if (!elem->renderData.verticesC
        || (transparent && !elem->transparent)
        || (!transparent && !elem->opaque)) return;

    xElementInstance &instance = modelInstance.elementInstanceP[elem->id];
    xMatrix mtxTrasformation = elem->matrix * modelInstance.location;
    if (!FOV.CheckSphere(mtxTrasformation.preTransformP(instance.bsCenter), instance.bsRadius) ||
        !FOV.CheckBox(instance.bbBox.TransformatedPoints(mtxTrasformation)) )
    {
        ++Performance.CulledElements;
        return;
    }

    /************************* INIT VBO ****************************/
    if (instance.mode == xGPURender::NONE)
        xRenderGL::InitVBO(elem);

    /************************* LOAD VERTICES ****************************/
    glPushMatrix();
    glMultMatrixf(&elem->matrix.x0);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
    if (elem->skeletized) {
        GLShader::EnableSkeleton(xState_On);
        GLShader::Start();
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones  (modelInstance.bonesC, modelInstance.bonesM, modelInstance.bonesQ, elem, true);
        g_AnimSkeletal.SetElement(elem, &instance, true);
    }
    else
        glVertexPointer   (3, GL_FLOAT, (elem->textured) ? sizeof(xVertexTex) : sizeof(xVertex), 0);

    /************************* RENDER FACES ****************************/
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.indexB );

    if ((transparent && !elem->opaque) ||
        (!transparent && !elem->transparent))
        glDrawElements (GL_TRIANGLES, 3*elem->facesC, GL_UNSIGNED_SHORT, 0);
    else
    {
        xFaceList *faceL = elem->faceListP;
        for(int i=elem->faceListC; i; --i, ++faceL)
        {
            if ((transparent && (!faceL->materialP || faceL->materialP->transparency == 0.f)) ||
                (!transparent && faceL->materialP && faceL->materialP->transparency > 0.f) )
                continue;
            glDrawElements(GL_TRIANGLES, 3*faceL->indexCount, GL_UNSIGNED_SHORT, (void*)(faceL->indexOffset*3*sizeof(xWORD)));
        }
    }
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB ( GL_ARRAY_BUFFER_ARB, 0 );

    if (elem->skeletized)
        g_AnimSkeletal.EndAnimation();
    GLShader::EnableSkeleton(xState_Off);

    glPopMatrix();
}

void xRenderGL :: RenderDepth( xModel &model, xModelInstance &instance,
                               bool transparent, const xFieldOfView &FOV )
{
    if ((transparent  && !model.transparent) ||
        (!transparent && !model.opaque)) return;

    glEnableClientState(GL_VERTEX_ARRAY);

    glPushMatrix();
    glMultMatrixf(&instance.location.x0);

    for (xElement *elem = model.kidsP; elem; elem = elem->nextP)
        if (UseVBO)
            RenderElementDepthVBO(elem, instance, transparent, FOV);
        else
            RenderElementDepthLST(elem, instance, transparent, FOV);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
}
