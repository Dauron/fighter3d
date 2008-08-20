#include "RendererGL.h"
#include "../GLAnimSkeletal.h"

/********************************* vertices && element selections ************************************/
void RenderElementDepthLST(
                            xElement * elem, xModelInstance &modelInstance,
                            bool transparent, const xFieldOfView &FOV )
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderElementDepthLST( selem, modelInstance, transparent, FOV );

    if (!elem->renderData.I_vertices
        || (transparent && !elem->FL_transparent)
        || (!transparent && !elem->FL_opaque)) return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];
    xMatrix mtxTrasformation = elem->MX_MeshToLocal * modelInstance.MX_LocalToWorld;
    if (!FOV.CheckSphere(mtxTrasformation.preTransformP(instance.bsCenter), instance.bsRadius) ||
        !FOV.CheckBox(instance.bbBox, mtxTrasformation) )
    {
        ++Performance.CulledElements;
        return;
    }
    
    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.x0);

    if (elem->FL_skeletized) {
        GLShader::EnableSkeleton(xState_On);
        GLShader::Start();
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones  (modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones, elem, false);
        g_AnimSkeletal.SetElement(elem, &instance, false);
    }
    else
        glVertexPointer   (3, GL_FLOAT, (elem->FL_textured) ? sizeof(xVertexTex) : sizeof(xVertex), elem->renderData.L_vertices);

    /************************* RENDER FACES ****************************/
    if ((transparent && !elem->FL_opaque) ||
        (!transparent && !elem->FL_transparent))
        glDrawElements (GL_TRIANGLES, 3*elem->I_faces, GL_UNSIGNED_SHORT, elem->renderData.L_faces);
    else
    {
        xFaceList *faceL = elem->L_faceLists;
        for(int i=elem->I_faceLists; i; --i, ++faceL)
        {
            if ((transparent && (!faceL->Material || faceL->Material->transparency == 0.f)) ||
                (!transparent && faceL->Material && faceL->Material->transparency > 0.f) )
                continue;
            glDrawElements(GL_TRIANGLES, 3*faceL->I_count, GL_UNSIGNED_SHORT, elem->renderData.L_faces+faceL->I_offset);
        }
    }

    if (elem->FL_skeletized)
        g_AnimSkeletal.EndAnimation();
    GLShader::EnableSkeleton(xState_Off);

    glPopMatrix();
}

void RenderElementDepthVBO( xElement * elem, xModelInstance &modelInstance,
                            bool transparent, const xFieldOfView &FOV )
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderElementDepthVBO( selem, modelInstance, transparent, FOV );

    if (!elem->renderData.I_vertices
        || (transparent && !elem->FL_transparent)
        || (!transparent && !elem->FL_opaque)) return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];
    xMatrix mtxTrasformation = elem->MX_MeshToLocal * modelInstance.MX_LocalToWorld;
    if (!FOV.CheckSphere(mtxTrasformation.preTransformP(instance.bsCenter), instance.bsRadius) ||
        !FOV.CheckBox(instance.bbBox, mtxTrasformation) )
    {
        ++Performance.CulledElements;
        return;
    }

    /************************* INIT VBO ****************************/
    if (instance.mode == xGPURender::NONE)
        RendererGL::InitVBO(elem);

    /************************* LOAD VERTICES ****************************/
    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.x0);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
    if (elem->FL_skeletized) {
        GLShader::EnableSkeleton(xState_On);
        GLShader::Start();
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones  (modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones, elem, true);
        g_AnimSkeletal.SetElement(elem, &instance, true);
    }
    else
        glVertexPointer   (3, GL_FLOAT, (elem->FL_textured) ? sizeof(xVertexTex) : sizeof(xVertex), 0);

    /************************* RENDER FACES ****************************/
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.indexB );

    if ((transparent && !elem->FL_opaque) ||
        (!transparent && !elem->FL_transparent))
        glDrawElements (GL_TRIANGLES, 3*elem->I_faces, GL_UNSIGNED_SHORT, 0);
    else
    {
        xFaceList *faceL = elem->L_faceLists;
        for(int i=elem->I_faceLists; i; --i, ++faceL)
        {
            if ((transparent && (!faceL->Material || faceL->Material->transparency == 0.f)) ||
                (!transparent && faceL->Material && faceL->Material->transparency > 0.f) )
                continue;
            glDrawElements(GL_TRIANGLES, 3*faceL->I_count, GL_UNSIGNED_SHORT, (void*)(faceL->I_offset*3*sizeof(xWORD)));
        }
    }
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB ( GL_ARRAY_BUFFER_ARB, 0 );

    if (elem->FL_skeletized)
        g_AnimSkeletal.EndAnimation();
    GLShader::EnableSkeleton(xState_Off);

    glPopMatrix();
}

void RendererGL :: RenderDepth( xModel &model, xModelInstance &instance,
                               bool transparent, const xFieldOfView &FOV )
{
    if ((transparent  && !model.FL_transparent) ||
        (!transparent && !model.FL_opaque)) return;

    glEnableClientState(GL_VERTEX_ARRAY);

    glPushMatrix();
    glMultMatrixf(&instance.MX_LocalToWorld.x0);

    for (xElement *elem = model.L_kids; elem; elem = elem->Next)
        if (UseVBO)
            RenderElementDepthVBO(elem, instance, transparent, FOV);
        else
            RenderElementDepthLST(elem, instance, transparent, FOV);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
}
