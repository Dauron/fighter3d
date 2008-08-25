#include "RendererGL.h"
#include "../GLAnimSkeletal.h"

/********************************* shadows ************************************/
void RenderShadowMapLST(xElement *elem, xModelInstance &modelInstance,
                        const Math::Cameras::FieldOfView &FOV)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderShadowMapLST(selem, modelInstance, FOV);

    if (!elem->renderData.I_vertices) return;

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
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.x0);

    if (elem->FL_skeletized) {
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones (modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones, elem, false);
        g_AnimSkeletal.SetElement(elem, &instance);
    }
    else
    {
        size_t stride = elem->FL_textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer   ( 3, GL_FLOAT, stride, elem->renderData.L_vertices );
        glTexCoordPointer ( 3, GL_FLOAT, stride, elem->renderData.L_vertices );
        glNormalPointer   ( GL_FLOAT, sizeof(xVector3), elem->renderData.L_normals );
    }

    glDrawElements(GL_TRIANGLES, 3*elem->I_faces, GL_UNSIGNED_SHORT, elem->renderData.L_faces);

    if (elem->FL_skeletized) g_AnimSkeletal.EndAnimation();

    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
}

void RenderShadowMapVBO(xElement *elem, xModelInstance &modelInstance,
                        const Math::Cameras::FieldOfView &FOV)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderShadowMapVBO(selem, modelInstance, FOV);

    if (!elem->renderData.I_vertices) return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];
    xMatrix mtxTrasformation = elem->MX_MeshToLocal * modelInstance.MX_LocalToWorld;
    if (!FOV.CheckSphere(mtxTrasformation.preTransformP(instance.bsCenter), instance.bsRadius) ||
        !FOV.CheckBox(instance.bbBox, mtxTrasformation) )
    {
        ++Performance.CulledElements;
        return;
    }
    if (instance.mode == xGPURender::NONE)
        RendererGL::InitVBO(elem);

    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.x0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.x0);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
    if (elem->FL_skeletized) {
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones  (modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones, elem, true);
        g_AnimSkeletal.SetElement(elem, &instance, true);
    }
    else
    {
        size_t stride = elem->FL_textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer   (3, GL_FLOAT, stride, 0);
        glTexCoordPointer (3, GL_FLOAT, stride, 0);
        glBindBufferARB ( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.normalB );
        glNormalPointer ( GL_FLOAT, sizeof(xVector3), 0 );
    }

    /************************* RENDER FACES ****************************/
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.indexB );
    glDrawElements  ( GL_TRIANGLES, 3*elem->I_faces, GL_UNSIGNED_SHORT, 0);

    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB ( GL_ARRAY_BUFFER_ARB, 0 );

    if (elem->FL_skeletized) g_AnimSkeletal.EndAnimation();

    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
}

void RendererGL :: RenderShadowMap(xModel &model, xModelInstance &instance,
                                   const xShadowMap &shadowMap,
                                   const Math::Cameras::FieldOfView &FOV )
{
    State::RenderingShadows = true;

    glBindTexture(GL_TEXTURE_2D, shadowMap.texId );
    GLShader::EnableTexturing(xState_On);
    GLShader::SetLightType(xLight_NONE);

    glEnable(GL_BLEND);
    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_POLYGON_SMOOTH);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);

    glPushMatrix();
    glMultMatrixf(&instance.MX_LocalToWorld.x0);
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    xMatrix vMatrix = instance.MX_LocalToWorld * shadowMap.receiverUVMatrix;
    glLoadMatrixf(&vMatrix.x0);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    for (xElement *elem = model.L_kids; elem; elem = elem->Next)
        if (UseVBO)
            RenderShadowMapVBO(elem, instance, FOV);
        else
            RenderShadowMapLST(elem, instance, FOV);

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glDisable(GL_BLEND);

    State::RenderingShadows = false;
}

/******************************* shadow maps **********************************/
void RenderShadowMapTextureLST(xElement *elem, xModelInstance &modelInstance, bool transparent)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderShadowMapTextureLST(selem, modelInstance, transparent);

    if (!elem->renderData.I_vertices
        || (transparent && !elem->FL_transparent)
        || (!transparent && !elem->FL_opaque)) return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];

    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.matrix[0][0]);

    if (elem->FL_skeletized) {
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones (modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones, elem, false);
        g_AnimSkeletal.SetElement(elem, &instance);
    }
    else
    {
        if (elem->FL_textured)
            glVertexPointer   (3, GL_FLOAT, sizeof(xVertexTex), elem->renderData.L_verticesT);
        else
            glVertexPointer   (3, GL_FLOAT, sizeof(xVertex), elem->renderData.L_vertices);
    }

    xFaceList *faceL = elem->L_faceLists;
    for(int i=elem->I_faceLists; i; --i, ++faceL)
    {
        if ((transparent && (!faceL->Material || faceL->Material->transparency == 0.f)) ||
            (!transparent && faceL->Material && faceL->Material->transparency > 0.f) )
            continue;
        glDrawElements(GL_TRIANGLES, 3*faceL->I_count, GL_UNSIGNED_SHORT, elem->renderData.L_faces+faceL->I_offset);
    }

    if (elem->FL_skeletized) g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

void RenderShadowMapTextureVBO(xElement *elem, xModelInstance &modelInstance, bool transparent)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderShadowMapTextureVBO(selem, modelInstance, transparent);

    if (!elem->renderData.I_vertices
        || (transparent  && !elem->FL_transparent)
        || (!transparent && !elem->FL_opaque)) return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];

    /************************* INIT VBO ****************************/
    if (instance.mode == xGPURender::NONE)
        RendererGL::InitVBO(elem);

    /************************* LOAD VERTICES ****************************/
    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.matrix[0][0]);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
    if (elem->FL_skeletized) {
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones  (modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones, elem, true);
        g_AnimSkeletal.SetElement(elem, &instance, true);
    }
    else
    {
        if (elem->FL_textured)
            glVertexPointer   (3, GL_FLOAT, sizeof(xVertexTex), 0);
        else
            glVertexPointer   (3, GL_FLOAT, sizeof(xVertex), 0);
    }

    /************************* RENDER FACES ****************************/
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.indexB );
    xFaceList *faceL = elem->L_faceLists;
    for(int i=elem->I_faceLists; i; --i, ++faceL)
    {
        if ((transparent && (!faceL->Material || faceL->Material->transparency == 0.f)) ||
            (!transparent && faceL->Material && faceL->Material->transparency > 0.f) )
            continue;
        glDrawElements(GL_TRIANGLES, 3*faceL->I_count, GL_UNSIGNED_SHORT, (void*)(faceL->I_offset*3*sizeof(xWORD)));
    }
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB ( GL_ARRAY_BUFFER_ARB, 0 );

    if (elem->FL_skeletized) g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

void RenderShadowMapTexture(xModel &model, xModelInstance &instance, bool transparent, bool useVBO)
{
    if ((transparent  && !model.FL_transparent) ||
        (!transparent && !model.FL_opaque)) return;

    glEnableClientState(GL_VERTEX_ARRAY);

    for (xElement *elem = model.L_kids; elem; elem = elem->Next)
        if (useVBO)
            RenderShadowMapTextureVBO(elem, instance, transparent);
        else
            RenderShadowMapTextureLST(elem, instance, transparent);

    glDisableClientState(GL_VERTEX_ARRAY);
}
void RendererGL :: CreateShadowMapTexture(xModel &model, xModelInstance &instance, xDWORD &shadowMapTexId,
                                         xWORD width, xMatrix &mtxBlockerToLight)
{
    glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT | GL_TRANSFORM_BIT);
    glViewport(0, 0, width, width);
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, width, width);

    glMatrixMode(GL_PROJECTION);
    // Save OpenGL's old PROJECTION matrix
    glPushMatrix(); //glGetFloatv (GL_MODELVIEW_MATRIX, &mtxProjection.x0);
    // Clear OpenGL's PROJECTION matrix
    glLoadIdentity();

    // We will make a dark grey on white shadow-map,
    // so clear the buffer with white
    glClearColor(1.0, 1.0, 1.0, 0.0);

    glDisable(GL_BLEND);
    GLShader::SetLightType(xLight_NONE);
    GLShader::EnableTexturing(xState_Disable);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Load BlockerLocalToShadowMap matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&mtxBlockerToLight.x0);

    // DRAW
    // Shadow color
    glColor3f(0.5f, 0.5f, 0.5f);
    RenderShadowMapTexture(model, instance, false, UseVBO);
    // Shadow color
    glColor3f(0.9f, 0.9f, 0.9f);
    RenderShadowMapTexture(model, instance, true, UseVBO);

    GLShader::EnableTexturing(xState_Enable);

    glPopMatrix();
    // Restore OpenGL's PROJECTION matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();//glLoadMatrixf(&mtxProjection.x0);
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();

    // Make the image we just rendered (the shadow-map) an OpenGL texture
    if (!shadowMapTexId)
    {
        GLuint tid;
        glGenTextures(1, &tid);
        shadowMapTexId = tid;
        glBindTexture(GL_TEXTURE_2D, shadowMapTexId);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        float Sh_TxBorder[4]= { 1.0, 1.0, 1.0, 1.0};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, Sh_TxBorder);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
        glBindTexture(GL_TEXTURE_2D, shadowMapTexId);
    glReadBuffer(GL_BACK);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8/*GL_INTENSITY8*/, 0, 0, width, width, 0);
}
