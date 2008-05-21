#include "xRenderGL.h"
#include "xShadowVolume.h"
#include "../../GLExtensions/EXT_stencil_wrap.h"
#include "../../GLExtensions/EXT_stencil_two_side.h"
#include "../../OpenGL/GLAnimSkeletal.h"

/********************************* shadows ************************************/
void xRenderGL :: RenderShadowMap(const xShadowMap &shadowMap, const xFieldOfView *FOV )
{
    assert(xModelToRender);

    if (!bonesC && spineP)
        CalculateSkeleton();
    if (!instanceDataTRP)
        PrepareInstanceDataTr();

    this->FOV = FOV;

    State::RenderingShadows = true;

    glBindTexture(GL_TEXTURE_2D, shadowMap.texId );
    GLShader::EnableTexturing(1);
    GLShader::EnableLighting(-1);

    glEnable(GL_BLEND);
    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_POLYGON_SMOOTH);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);

    glPushMatrix();
    glMultMatrixf(&location.x0);
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    xMatrix vMatrix = location * shadowMap.receiverUVMatrix;
    glLoadMatrixf(&vMatrix.x0);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    for (xElement *elem = xModelToRender->kidsP; elem; elem = elem->nextP)
        if (UseVBO)
            RenderShadowMapVBO(elem);
        else
            RenderShadowMapLST(elem);

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glDisable(GL_BLEND);

    State::RenderingShadows = false;
}

void xRenderGL :: RenderShadowMapLST(xElement *elem)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderShadowMapLST(selem);

    if (!elem->renderData.verticesC) return;

    xElementInstance *instance = instanceDataTRP + elem->id;
    xMatrix mtxTrasformation = elem->matrix * location;
    if (FOV &&
        (!FOV->CheckSphere(mtxTrasformation.preTransformP(instance->bsCenter), instance->bsRadius) ||
         !FOV->CheckBox(instance->bbBox.TransformatedPoints(elem->matrix * location)) ) )
    {
        ++Performance.CulledElements;
        return;
    }

    glPushMatrix();
    glMultMatrixf(&elem->matrix.x0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(&elem->matrix.x0);

    if (elem->skeletized) {
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones (bonesC, bonesM, bonesQ);
        g_AnimSkeletal.SetElement(elem, instance);
    }
    else
    {
        size_t stride = elem->textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer   ( 3, GL_FLOAT, stride, elem->renderData.verticesP );
        glTexCoordPointer ( 3, GL_FLOAT, stride, elem->renderData.verticesP );
        glNormalPointer   ( GL_FLOAT, sizeof(xVector3), elem->renderData.normalP );
    }

    glDrawElements(GL_TRIANGLES, 3*elem->facesC, GL_UNSIGNED_SHORT, elem->renderData.facesP);

    if (elem->skeletized) g_AnimSkeletal.EndAnimation();

    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
}

void xRenderGL :: RenderShadowMapVBO(xElement *elem)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderShadowMapVBO(selem);

    if (!elem->renderData.verticesC) return;

    xElementInstance *instance = instanceDataTRP + elem->id;
    xMatrix mtxTrasformation = elem->matrix * location;
    if (FOV &&
        (!FOV->CheckSphere(mtxTrasformation.preTransformP(instance->bsCenter), instance->bsRadius) ||
         !FOV->CheckBox(instance->bbBox.TransformatedPoints(elem->matrix * location)) ) )
    {
        ++Performance.CulledElements;
        return;
    }
    if (instance->mode == xElementInstance::xRenderMode_NULL)
        InitVBO(elem, instance);

    glPushMatrix();
    glMultMatrixf(&elem->matrix.x0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(&elem->matrix.x0);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, instance->gpuMain.vertexB );
    if (elem->skeletized) {
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones  (bonesC, bonesM, bonesQ);
        g_AnimSkeletal.SetElement(elem, instance, true);
    }
    else
    {
        size_t stride = elem->textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer   (3, GL_FLOAT, stride, 0);
        glTexCoordPointer (3, GL_FLOAT, stride, 0);
        glBindBufferARB ( GL_ARRAY_BUFFER_ARB, instance->gpuMain.normalB );
        glNormalPointer ( GL_FLOAT, sizeof(xVector3), 0 );
    }

    /************************* RENDER FACES ****************************/
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, instance->gpuMain.indexB );
    glDrawElements  ( GL_TRIANGLES, 3*elem->facesC, GL_UNSIGNED_SHORT, 0);

    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB ( GL_ARRAY_BUFFER_ARB, 0 );

    if (elem->skeletized) g_AnimSkeletal.EndAnimation();

    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
}

/******************************* shadow maps **********************************/
void   xRenderGL :: RenderShadowMapTexture(bool transparent)
{
    assert(xModelToRender);

    if (!bonesC && spineP)
        CalculateSkeleton();
    if (!instanceDataTRP)
        PrepareInstanceDataTr();

    glEnableClientState(GL_VERTEX_ARRAY);

    for (xElement *elem = xModelToRender->kidsP; elem; elem = elem->nextP)
        // NOTE: MIX of display lists and VBO appears to be much slower than VBO only
        if (UseVBO)
            RenderShadowMapTextureVBO(elem, transparent);
        else
            RenderShadowMapTextureLST(elem, transparent);

    glDisableClientState(GL_VERTEX_ARRAY);
}

void   xRenderGL :: RenderShadowMapTextureLST(xElement *elem, bool transparent)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderShadowMapTextureLST(selem, transparent);

    if (!elem->renderData.verticesC
        || (transparent && !elem->renderData.transparent)
        || (!transparent && !elem->renderData.opaque)) return;

    xElementInstance *instance = instanceDataTRP + elem->id;
    xDWORD &listID = transparent ? instance->gpuMain.listIDTransp : instance->gpuMain.listID;

    glPushMatrix();
    glMultMatrixf(&elem->matrix.matrix[0][0]);

    if (elem->skeletized) {
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones (bonesC, bonesM, bonesQ);
        g_AnimSkeletal.SetElement(elem, instance);
    }
    else
    {
        if (elem->textured)
            glVertexPointer   (3, GL_FLOAT, sizeof(xVertexTex), elem->renderData.verticesTP);
        else
            glVertexPointer   (3, GL_FLOAT, sizeof(xVertex), elem->renderData.verticesP);
    }

    xFaceList *faceL = elem->faceListP;
    for(int i=elem->faceListC; i; --i, ++faceL)
    {
        if ((transparent && (!faceL->materialP || faceL->materialP->transparency == 0.f)) ||
            (!transparent && faceL->materialP && faceL->materialP->transparency > 0.f) )
            continue;
        glDrawElements(GL_TRIANGLES, 3*faceL->indexCount, GL_UNSIGNED_SHORT, elem->renderData.facesP+faceL->indexOffset);
    }

    if (elem->skeletized) g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

void   xRenderGL :: RenderShadowMapTextureVBO(xElement *elem, bool transparent)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderShadowMapTextureVBO(selem, transparent);

    if (!elem->renderData.verticesC
        || (transparent && !elem->renderData.transparent)
        || (!transparent && !elem->renderData.opaque)) return;

    xElementInstance *instance = instanceDataTRP + elem->id;

    /************************* INIT VBO ****************************/
    if (instance->mode == xElementInstance::xRenderMode_NULL)
        InitVBO(elem, instance);

    /************************* LOAD VERTICES ****************************/
    glPushMatrix();
    glMultMatrixf(&elem->matrix.matrix[0][0]);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, instance->gpuMain.vertexB );
    if (elem->skeletized) {
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones  (bonesC, bonesM, bonesQ);
        g_AnimSkeletal.SetElement(elem, instance, true);
    }
    else
    {
        if (elem->textured)
            glVertexPointer   (3, GL_FLOAT, sizeof(xVertexTex), 0);
        else
            glVertexPointer   (3, GL_FLOAT, sizeof(xVertex), 0);
    }

    /************************* RENDER FACES ****************************/
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, instance->gpuMain.indexB );
    xFaceList *faceL = elem->faceListP;
    for(int i=elem->faceListC; i; --i, ++faceL)
    {
        if ((transparent && (!faceL->materialP || faceL->materialP->transparency == 0.f)) ||
            (!transparent && faceL->materialP && faceL->materialP->transparency > 0.f) )
            continue;
        glDrawElements(GL_TRIANGLES, 3*faceL->indexCount, GL_UNSIGNED_SHORT, (void*)(faceL->indexOffset*3*sizeof(xWORD)));
    }
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB ( GL_ARRAY_BUFFER_ARB, 0 );

    if (elem->skeletized) g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

xDWORD xRenderGL :: CreateShadowMapTexture(xWORD width, xMatrix &mtxBlockerToLight)
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
    GLShader::EnableLighting(0);
    GLShader::EnableTexturing(0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Load BlockerLocalToShadowMap matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&mtxBlockerToLight.x0);

    // DRAW
    xModel * xModelToRenderOld = xModelToRender;
    xModelToRender = xModelPhysical;
    if (xModelToRenderOld != xModelPhysical)
        SetRenderMode(xRender::rmPhysical);
    // Shadow color
    glColor3f(0.5f, 0.5f, 0.5f);
    this->RenderShadowMapTexture(false);
    // Shadow color
    glColor3f(0.9f, 0.9f, 0.9f);
    this->RenderShadowMapTexture(true);
    //xModelToRender = xModelToRenderOld;
    if (xModelToRenderOld != xModelPhysical)
        SetRenderMode(xRender::rmGraphical);

    glPopMatrix();
    // Restore OpenGL's PROJECTION matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();//glLoadMatrixf(&mtxProjection.x0);
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();

    // Make the image we just rendered (the shadow-map) an OpenGL texture
    /*
    if (width != this->shadowWidth)
    {
        if (this->texture) delete[] this->texture;
        this->texture = NULL;
        this->shadowWidth = width;
    }
    if (!this->texture)
        this->texture = new xDWORD[width*width*4];
    if(this->texture)
    {
        if (!this->shadowTexId)
            glGenTextures(1, (GLuint*)&this->shadowTexId);

        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, width, width, GL_RGBA, GL_UNSIGNED_BYTE, this->texture);
        // Send the shadow map to OpenGL
        glBindTexture(GL_TEXTURE_2D, this->shadowTexId);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        float Sh_TxBorder[4]= { 1.0, 1.0, 1.0, 1.0};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, Sh_TxBorder);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->texture );
    }
    */
    // NOTE: glCopyTexImage2D is MUCH faster than glReadPixels+glTexImage2D
    if (!this->shadowMapTexId)
    {
        glGenTextures(1, (GLuint*)&this->shadowMapTexId);
        glBindTexture(GL_TEXTURE_2D, this->shadowMapTexId);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        float Sh_TxBorder[4]= { 1.0, 1.0, 1.0, 1.0};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, Sh_TxBorder);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
        glBindTexture(GL_TEXTURE_2D, this->shadowMapTexId);
    glReadBuffer(GL_BACK);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8/*GL_INTENSITY8*/, 0, 0, width, width, 0);

    return this->shadowMapTexId;
}

/***************************** shadow volumes *********************************/
void xRenderGL :: RenderShadowVolume(xLight &light, xFieldOfView *FOV)
{
    assert(xModelToRender);
/*
    xModel * xModelToRenderOld = xModelToRender;
    xModelToRender = xModelPhysical;
    if (xModelToRenderOld != xModelPhysical)
        SetRenderMode(xRender::rmPhysical);
*/
    if (!bonesC && spineP)
        CalculateSkeleton();
    if (!instanceDataTRP)
        PrepareInstanceDataTr();

    this->FOV = FOV;

    glPushMatrix();
    glMultMatrixf(&location.x0);

    glEnableClientState(GL_VERTEX_ARRAY);

    for (xElement *elem = xModelToRender->kidsP; elem; elem = elem->nextP)
        RenderShadowVolumeElem(elem, light);

    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
/*
    if (xModelToRenderOld != xModelPhysical)
        SetRenderMode(xRender::rmGraphical);
*/
}
void xRenderGL :: RenderShadowVolumeElem(xElement *elem, xLight &light)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderShadowVolumeElem(selem, light);

    if (!elem->renderData.verticesC) return;
    if (!elem->edgesC) return;

    xElementInstance *instance = instanceDataTRP + elem->id;

    if (!light.elementReceivesLight((elem->matrix * location).preTransformP(instance->bsCenter), instance->bsRadius)) return;

    if (!xShadows_ViewportMaybeShadowed(elem, instance, location, FOV, light))
        RenderShadowVolumeZPass(elem, light);
    else
        RenderShadowVolumeZFail(elem, light);
}
void xRenderGL :: RenderShadowVolumeZPass(xElement *elem, xLight &light)
{
    xElementInstance *instance = instanceDataTRP + elem->id;

    glPushMatrix();
    glMultMatrixf(&elem->matrix.x0);

    xMatrix  mtxWorldToObject = (elem->matrix * location).invert();
    xVector3 lightPos;
    bool infiniteL = light.type == xLight_INFINITE;
    if (infiniteL)
        lightPos = mtxWorldToObject.preTransformV(light.position);
    else
        lightPos = mtxWorldToObject.preTransformP(light.position);

    bool              *facingFlag = NULL;
    xRenderShadowData  shadowData; shadowData.indexP = NULL;
    bool               useBackCapOptimization
        = (lightPos - instance->bsCenter).lengthSqr() > instance->bsRadius*instance->bsRadius;

    shadowData.verticesP = NULL;
    shadowData.normalsP  = NULL;
    shadowData.indexP    = NULL;
    xElement_GetSkinnedElementForShadow(elem, bonesM, infiniteL, shadowData);
    xShadows_ExtrudePoints(elem, infiniteL, lightPos, shadowData);
    xShadows_GetBackFaces (elem, infiniteL, shadowData, facingFlag);
    xShadows_GetSilhouette(elem, infiniteL, useBackCapOptimization, facingFlag, shadowData);

    glVertexPointer   (4, GL_FLOAT, sizeof(xVector4), shadowData.verticesP);

    /************************* RENDER FACES ****************************/

    if (GLExtensions::Exists_EXT_StencilTwoSide)
    {
        glActiveStencilFaceEXT(GL_BACK);
        if (GLExtensions::Exists_EXT_StencilWrap)
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);
        else
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
        glActiveStencilFaceEXT(GL_FRONT);
        if (GLExtensions::Exists_EXT_StencilWrap)
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
        else
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

        // Extruded quads
        if (!infiniteL)
            glDrawElements ( GL_QUADS, 4*shadowData.sideC, GL_UNSIGNED_SHORT, shadowData.indexP);
        else
            glDrawElements ( GL_TRIANGLES, 3*shadowData.sideC, GL_UNSIGNED_SHORT, shadowData.indexP);
    }
    else
    {
        glCullFace(GL_FRONT);
        if (GLExtensions::Exists_EXT_StencilWrap)
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
        else
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
        glColor4f(1.f, 0.f, 0.f, 0.4f);

        // Extruded quads
        if (!infiniteL)
            glDrawElements ( GL_QUADS, 4*shadowData.sideC, GL_UNSIGNED_SHORT, shadowData.indexP);
        else
            glDrawElements ( GL_TRIANGLES, 3*shadowData.sideC, GL_UNSIGNED_SHORT, shadowData.indexP);

        glCullFace(GL_BACK);
        if (GLExtensions::Exists_EXT_StencilWrap)
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);
        else
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
        glColor4f(0.f, 1.f, 0.f, 0.4f);

        // Extruded quads
        if (!infiniteL)
            glDrawElements ( GL_QUADS, 4*shadowData.sideC, GL_UNSIGNED_SHORT, shadowData.indexP);
        else
            glDrawElements ( GL_TRIANGLES, 3*shadowData.sideC, GL_UNSIGNED_SHORT, shadowData.indexP);
    }

    delete[] facingFlag;
    delete[] shadowData.indexP;
    delete[] shadowData.verticesP;
    if (shadowData.normalsP)
        delete[] shadowData.normalsP;

    glPopMatrix();
}
void xRenderGL :: RenderShadowVolumeZFail(xElement *elem, xLight &light)
{
    xElementInstance *instance = instanceDataTRP + elem->id;

    glPushMatrix();
    glMultMatrixf(&elem->matrix.x0);

    xMatrix  mtxWorldToObject = (elem->matrix * location).invert();
    xVector3 lightPos;
    bool infiniteL = light.type == xLight_INFINITE;
    if (infiniteL)
        lightPos = mtxWorldToObject.preTransformV(light.position);
    else
        lightPos = mtxWorldToObject.preTransformP(light.position);

    bool              *facingFlag = NULL;
    xRenderShadowData  shadowData; shadowData.indexP = NULL;
    bool               useBackCapOptimization
        = (lightPos - instance->bsCenter).lengthSqr() > instance->bsRadius*instance->bsRadius;

    shadowData.verticesP = NULL;
    shadowData.normalsP  = NULL;
    shadowData.indexP    = NULL;
    xElement_GetSkinnedElementForShadow(elem, bonesM, infiniteL, shadowData);
    xShadows_ExtrudePoints(elem, infiniteL, lightPos, shadowData);
    xShadows_GetBackFaces (elem, infiniteL, shadowData, facingFlag);
    xShadows_GetSilhouette(elem, infiniteL, useBackCapOptimization, facingFlag, shadowData);

    glVertexPointer   (4, GL_FLOAT, sizeof(xVector4), shadowData.verticesP);

    /************************* RENDER FACES ****************************/

    if (GLExtensions::Exists_EXT_StencilTwoSide)
    {
        glColor4f(1.f, 0.f, 0.f, 0.4f);
        glActiveStencilFaceEXT(GL_BACK);
        if (GLExtensions::Exists_EXT_StencilWrap)
            glStencilOp(GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
        else
            glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
        glActiveStencilFaceEXT(GL_FRONT);
        if (GLExtensions::Exists_EXT_StencilWrap)
            glStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
        else
            glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);

        if (!infiniteL)
        {
            // Extruded quads
            glDrawElements ( GL_QUADS, 4*shadowData.sideC, GL_UNSIGNED_SHORT, shadowData.indexP);
            // Front cap
            glDrawElements ( GL_TRIANGLES, 3*shadowData.frontC, GL_UNSIGNED_SHORT,
                shadowData.indexP + 4*shadowData.sideC);
            // Back cap
            glDrawElements ( GL_TRIANGLES, 3*shadowData.backC, GL_UNSIGNED_SHORT,
                shadowData.indexP + 4*shadowData.sideC + 3*shadowData.frontC);
        }
        else
        {
            // Extruded tris
            glDrawElements ( GL_TRIANGLES, 3*shadowData.sideC, GL_UNSIGNED_SHORT, shadowData.indexP);
            // Front cap
            glDrawElements ( GL_TRIANGLES, 3*shadowData.frontC, GL_UNSIGNED_SHORT,
                shadowData.indexP + 3*shadowData.sideC);
        }
    }
    else
    {
        glCullFace(GL_BACK);
        if (GLExtensions::Exists_EXT_StencilWrap)
            glStencilOp(GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
        else
            glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
        glColor4f(1.f, 0.f, 0.f, 0.4f);

        if (!infiniteL)
        {
            // Extruded quads
            glDrawElements ( GL_QUADS, 4*shadowData.sideC, GL_UNSIGNED_SHORT, shadowData.indexP);
            // Front cap
            glDrawElements ( GL_TRIANGLES, 3*shadowData.frontC, GL_UNSIGNED_SHORT,
                shadowData.indexP + 4*shadowData.sideC);
            // Back cap
            glDrawElements ( GL_TRIANGLES, 3*shadowData.backC, GL_UNSIGNED_SHORT,
                shadowData.indexP + 4*shadowData.sideC + 3*shadowData.frontC);
        }
        else
        {
            // Extruded tris
            glDrawElements ( GL_TRIANGLES, 3*shadowData.sideC, GL_UNSIGNED_SHORT, shadowData.indexP);
            // Front cap
            glDrawElements ( GL_TRIANGLES, 3*shadowData.frontC, GL_UNSIGNED_SHORT,
                shadowData.indexP + 3*shadowData.sideC);
        }

        glCullFace(GL_FRONT);
        if (GLExtensions::Exists_EXT_StencilWrap)
            glStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
        else
            glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
        glColor4f(0.f, 1.f, 0.f, 0.4f);

        if (!infiniteL)
        {
            // Extruded quads
            glDrawElements ( GL_QUADS, 4*shadowData.sideC, GL_UNSIGNED_SHORT, shadowData.indexP);
            // Front cap
            glDrawElements ( GL_TRIANGLES, 3*shadowData.frontC, GL_UNSIGNED_SHORT,
                shadowData.indexP + 4*shadowData.sideC);
            // Back cap
            glDrawElements ( GL_TRIANGLES, 3*shadowData.backC, GL_UNSIGNED_SHORT,
                shadowData.indexP + 4*shadowData.sideC + 3*shadowData.frontC);
        }
        else
        {
            // Extruded tris
            glDrawElements ( GL_TRIANGLES, 3*shadowData.sideC, GL_UNSIGNED_SHORT, shadowData.indexP);
            // Front cap
            glDrawElements ( GL_TRIANGLES, 3*shadowData.frontC, GL_UNSIGNED_SHORT,
                shadowData.indexP + 3*shadowData.sideC);
        }
    }

    delete[] facingFlag;
    delete[] shadowData.indexP;
    delete[] shadowData.verticesP;
    if (shadowData.normalsP)
        delete[] shadowData.normalsP;

    glPopMatrix();
}
