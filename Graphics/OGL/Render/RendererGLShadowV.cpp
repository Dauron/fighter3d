#include "RendererGL.h"

#include "../../../Config.h"
#include "../../ShadowVolume.h"
#include "../Extensions/EXT_stencil_wrap.h"
#include "../Extensions/EXT_stencil_two_side.h"

/***************************** shadow volumes *********************************/
void CheckShadowVolumeInFrustum(const xMatrix &transformation, const xMatrix &transformationInv,
                                xElementInstance &instance, xShadowData &shadowData,
                                const xLight &light, const xFieldOfView &FOV,
                                bool &outModelInFrustum, bool &outExtrusionInFrustum, bool &outBackCapInFrustum)
{
    outModelInFrustum = FOV.CheckSphere(transformation.preTransformP(instance.bsCenter), instance.bsRadius)
        &&  FOV.CheckBox(instance.bbBox.fillCornersTransformed(transformation));

    xVector4 bounds[16];
    for (int i = 0; i < 8; ++i)
        bounds[i].init(instance.bbBox.P_corners[i], 1.f);

    if (light.type != xLight_INFINITE)
    {
        for (int i = 0; i < 8; ++i)
            bounds[i+8].init ( instance.bbBox.P_corners[i] - light.position, 0.f );

        if (shadowData.zDataLevel == xShadowData::ZFAIL_PASS)
            outBackCapInFrustum = outModelInFrustum || outBackCapInFrustum || FOV.CheckPoints(bounds + 8, 8);
        else
            outBackCapInFrustum = false;

        outExtrusionInFrustum = FOV.CheckPoints(bounds, 16);
    }
    else
    {
        bounds[8].init(-light.position, 0.f);
        outExtrusionInFrustum = outModelInFrustum || FOV.CheckPoints(bounds, 9);
        outBackCapInFrustum = false;
    }
}

/*****************************      LST       *********************************/
void RenderShadowVolumeZPass(xShadowData &shadowData, bool infiniteL)
{
    /************************* RENDER FACES ****************************/
    if (!shadowData.gpuShadowPointers.listIDPass)
    {
        glNewList(shadowData.gpuShadowPointers.listIDPass = glGenLists(1), GL_COMPILE);
        {
            glVertexPointer   (4, GL_FLOAT, sizeof(xVector4), shadowData.L_vertices);
            // Extruded quads
            if (!infiniteL)
                glDrawElements ( GL_QUADS, 4*shadowData.I_sides, GL_UNSIGNED_SHORT, shadowData.L_indices);
            else
                glDrawElements ( GL_TRIANGLES, 3*shadowData.I_sides, GL_UNSIGNED_SHORT, shadowData.L_indices);
        }
        glEndList();
    }

    ++Performance.Shadows.zPass;
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
        glColor4f(1.f, 0.f, 0.f, 0.4f);
        glCallList(shadowData.gpuShadowPointers.listIDPass);
    }
    else
    {
        glCullFace(GL_FRONT);
        if (GLExtensions::Exists_EXT_StencilWrap)
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
        else
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
        glColor4f(1.f, 0.f, 0.f, 0.4f);
        glCallList(shadowData.gpuShadowPointers.listIDPass);

        glCullFace(GL_BACK);
        if (GLExtensions::Exists_EXT_StencilWrap)
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);
        else
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
        glColor4f(0.f, 1.f, 0.f, 0.4f);
        glCallList(shadowData.gpuShadowPointers.listIDPass);
    }
}
void RenderShadowVolumeZFail(xShadowData &shadowData, bool infiniteL, bool modelInFrustum, bool extrusionInFrustum, bool backCapInFrustum)
{
    /************************* RENDER FACES ****************************/
    if (modelInFrustum && backCapInFrustum && extrusionInFrustum)
    {
        if (!shadowData.gpuShadowPointers.listIDFail)
        {
            glNewList(shadowData.gpuShadowPointers.listIDFail = glGenLists(1), GL_COMPILE);
            {
                glVertexPointer   (4, GL_FLOAT, sizeof(xVector4), shadowData.L_vertices);
                if (infiniteL)// Extruded tris & Front cap
                    glDrawElements ( GL_TRIANGLES, 3*(shadowData.I_sides+shadowData.I_fronts), GL_UNSIGNED_SHORT, shadowData.L_indices);
                else
                {
                    // Extruded quads
                    glDrawElements ( GL_QUADS, 4*shadowData.I_sides, GL_UNSIGNED_SHORT, shadowData.L_indices);
                    // Front & Back cap
                    glDrawElements ( GL_TRIANGLES, 3*(shadowData.I_fronts+shadowData.I_backs), GL_UNSIGNED_SHORT,
                        shadowData.L_indices + 4*shadowData.I_sides);
                }
            }
            glEndList();
        }
    }
    else
    {
        if (modelInFrustum && !shadowData.gpuShadowPointers.listIDFailF)
        {
            glNewList(shadowData.gpuShadowPointers.listIDFailF = glGenLists(1), GL_COMPILE);
            {
                glVertexPointer   (4, GL_FLOAT, sizeof(xVector4), shadowData.L_vertices);
                if (infiniteL)
                    glDrawElements ( GL_TRIANGLES, 3*shadowData.I_fronts, GL_UNSIGNED_SHORT,
                        shadowData.L_indices + 3*shadowData.I_sides);
                else
                    glDrawElements ( GL_TRIANGLES, 3*shadowData.I_fronts, GL_UNSIGNED_SHORT,
                        shadowData.L_indices + 4*shadowData.I_sides);
            }
            glEndList();
        }
        if (extrusionInFrustum && !shadowData.gpuShadowPointers.listIDFailS) // Extruded quads
        {
            glNewList(shadowData.gpuShadowPointers.listIDFailS = glGenLists(1), GL_COMPILE);
            {
                glVertexPointer   (4, GL_FLOAT, sizeof(xVector4), shadowData.L_vertices);
                if (infiniteL)
                    glDrawElements ( GL_TRIANGLES, 3*shadowData.I_sides, GL_UNSIGNED_SHORT, shadowData.L_indices);
                else
                    glDrawElements ( GL_QUADS, 4*shadowData.I_sides, GL_UNSIGNED_SHORT, shadowData.L_indices);
            }
            glEndList();
        }
        if (backCapInFrustum && !infiniteL && !shadowData.gpuShadowPointers.listIDFailB) // Back cap
        {
            glNewList(shadowData.gpuShadowPointers.listIDFailB = glGenLists(1), GL_COMPILE);
            {
                glVertexPointer   (4, GL_FLOAT, sizeof(xVector4), shadowData.L_vertices);
                if (infiniteL)
                    glDrawElements ( GL_TRIANGLES, 3*shadowData.I_backs, GL_UNSIGNED_SHORT,
                        shadowData.L_indices + 3*shadowData.I_sides + 3*shadowData.I_fronts);
                else
                    glDrawElements ( GL_TRIANGLES, 3*shadowData.I_backs, GL_UNSIGNED_SHORT,
                        shadowData.L_indices + 4*shadowData.I_sides + 3*shadowData.I_fronts);
            }
            glEndList();
        }
    }

    if (!modelInFrustum && !extrusionInFrustum && !backCapInFrustum)
        ++Performance.Shadows.culled;

    if (GLExtensions::Exists_EXT_StencilTwoSide)
    {
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
        glColor4f(1.f, 0.f, 0.f, 0.4f);
        if (modelInFrustum && extrusionInFrustum && backCapInFrustum)
        {
            glCallList(shadowData.gpuShadowPointers.listIDFail);
            ++Performance.Shadows.zFail;
        }
        else
        {
            if (extrusionInFrustum)
            {
                glCallList(shadowData.gpuShadowPointers.listIDFailS);
                ++Performance.Shadows.zFailS;
            }
            if (modelInFrustum)
            {
                glCallList(shadowData.gpuShadowPointers.listIDFailF);
                ++Performance.Shadows.zFailF;
            }
            if (backCapInFrustum && !infiniteL)
            {
                glCallList(shadowData.gpuShadowPointers.listIDFailB);
                ++Performance.Shadows.zFailB;
            }
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
        if (modelInFrustum && extrusionInFrustum && backCapInFrustum)
        {
            glCallList(shadowData.gpuShadowPointers.listIDFail);
            ++Performance.Shadows.zFail;
        }
        else
        {
            if (extrusionInFrustum)
            {
                glCallList(shadowData.gpuShadowPointers.listIDFailS);
                ++Performance.Shadows.zFailS;
            }
            if (modelInFrustum)
            {
                glCallList(shadowData.gpuShadowPointers.listIDFailF);
                ++Performance.Shadows.zFailF;
            }
            if (backCapInFrustum && !infiniteL)
            {
                glCallList(shadowData.gpuShadowPointers.listIDFailB);
                ++Performance.Shadows.zFailB;
            }
        }

        glCullFace(GL_FRONT);
        if (GLExtensions::Exists_EXT_StencilWrap)
            glStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
        else
            glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
        glColor4f(0.f, 1.f, 0.f, 0.4f);
        if (modelInFrustum && extrusionInFrustum && backCapInFrustum)
        {
            glCallList(shadowData.gpuShadowPointers.listIDFail);
            ++Performance.Shadows.zFail;
        }
        else
        {
            if (extrusionInFrustum)
            {
                glCallList(shadowData.gpuShadowPointers.listIDFailS);
                ++Performance.Shadows.zFailS;
            }
            if (modelInFrustum)
            {
                glCallList(shadowData.gpuShadowPointers.listIDFailF);
                ++Performance.Shadows.zFailF;
            }
            if (backCapInFrustum && !infiniteL)
            {
                glCallList(shadowData.gpuShadowPointers.listIDFailB);
                ++Performance.Shadows.zFailB;
            }
        }
    }
}

void RenderShadowVolumeElem (xElement *elem, xModelInstance &modelInstance, xLight &light, xFieldOfView &FOV)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderShadowVolumeElem(selem, modelInstance, light, FOV);

    if (!elem->renderData.I_vertices) return;
    if (!elem->I_edges) return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];
    xMatrix mtxTrasformation = elem->MX_MeshToLocal * modelInstance.MX_LocalToWorld;
    xMatrix  mtxWorldToObject = (elem->MX_MeshToLocal * modelInstance.MX_LocalToWorld).invert();
    if (!light.elementReceivesLight(mtxTrasformation.preTransformP(instance.bsCenter), instance.bsRadius)) return;

    bool zPass = !ShadowVolume::ViewportMaybeShadowed(elem, instance, modelInstance.MX_LocalToWorld, FOV, light);
    bool infiniteL = light.type == xLight_INFINITE;
    xVector3 lightPos = (infiniteL)
        ? mtxWorldToObject.preTransformV(light.position)
        : mtxWorldToObject.preTransformP(light.position);

    xShadowData &shadowData = instance.GetShadowData(light, zPass ? xShadowData::ZPASS_ONLY : xShadowData::ZFAIL_PASS);
    if (!shadowData.L_indices)
    {
        bool *facingFlag = NULL;
        bool  useBackCapOptimization
            = (lightPos - instance.bsCenter).lengthSqr() > instance.bsRadius*instance.bsRadius;

        if (!shadowData.L_vertices)
            shadowData.L_vertices = (infiniteL) ? new xVector4[instance.I_vertices + 1] : new xVector4[instance.I_vertices << 1];
        memcpy (shadowData.L_vertices, instance.L_vertices, instance.I_vertices*sizeof(xVector4));
        ShadowVolume::ExtrudePoints(elem, infiniteL, lightPos, shadowData);
        ShadowVolume::GetBackFaces (elem, instance, infiniteL, shadowData, facingFlag);
        ShadowVolume::GetSilhouette(elem, infiniteL, useBackCapOptimization, facingFlag, shadowData);
        delete[] facingFlag;

        if (shadowData.gpuShadowPointers.listIDPass)
        {
            glDeleteLists(shadowData.gpuShadowPointers.listIDPass, 1);
            shadowData.gpuShadowPointers.listIDPass = 0;
        }
        if (shadowData.gpuShadowPointers.listIDFail)
        {
            glDeleteLists(shadowData.gpuShadowPointers.listIDFail, 1);
            shadowData.gpuShadowPointers.listIDFail = 0;
        }
        if (shadowData.gpuShadowPointers.listIDFailS)
        {
            glDeleteLists(shadowData.gpuShadowPointers.listIDFailS, 1);
            shadowData.gpuShadowPointers.listIDFailS = 0;
        }
        if (shadowData.gpuShadowPointers.listIDFailF)
        {
            glDeleteLists(shadowData.gpuShadowPointers.listIDFailF, 1);
            shadowData.gpuShadowPointers.listIDFailF = 0;
        }
        if (shadowData.gpuShadowPointers.listIDFailB)
        {
            glDeleteLists(shadowData.gpuShadowPointers.listIDFailB, 1);
            shadowData.gpuShadowPointers.listIDFailB = 0;
        }
    }

    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.x0);

    bool modelInFrustum, extrusionInFrustum, backCapInFrustum;
    CheckShadowVolumeInFrustum(mtxTrasformation, mtxWorldToObject,
        instance, shadowData, light, FOV,
        modelInFrustum, extrusionInFrustum, backCapInFrustum);

    ++Performance.Shadows.shadows;
    if (zPass)
    {
        if (extrusionInFrustum)
            RenderShadowVolumeZPass(shadowData, infiniteL);
        else
            ++Performance.Shadows.culled;
    }
    else
        RenderShadowVolumeZFail(shadowData, infiniteL, modelInFrustum, extrusionInFrustum, backCapInFrustum);

    glPopMatrix();
}

/*****************************      VBO       *********************************/
void RenderShadowVolumeZPassVBO(xShadowData &shadowData, bool infiniteL)
{
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, shadowData.gpuShadowPointers.vertexB );
    glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, shadowData.gpuShadowPointers.indexB );
    /************************* RENDER FACES ****************************/
    glVertexPointer   (4, GL_FLOAT, sizeof(xVector4), 0);

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
        glColor4f(1.f, 0.f, 0.f, 0.4f);

        // Extruded quads
        if (!infiniteL)
            glDrawElements ( GL_QUADS, 4*shadowData.I_sides, GL_UNSIGNED_SHORT, 0);
        else
            glDrawElements ( GL_TRIANGLES, 3*shadowData.I_sides, GL_UNSIGNED_SHORT, 0);
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
            glDrawElements ( GL_QUADS, 4*shadowData.I_sides, GL_UNSIGNED_SHORT, 0);
        else
            glDrawElements ( GL_TRIANGLES, 3*shadowData.I_sides, GL_UNSIGNED_SHORT, 0);

        glCullFace(GL_BACK);
        if (GLExtensions::Exists_EXT_StencilWrap)
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);
        else
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
        glColor4f(0.f, 1.f, 0.f, 0.4f);

        // Extruded quads
        if (!infiniteL)
            glDrawElements ( GL_QUADS, 4*shadowData.I_sides, GL_UNSIGNED_SHORT, 0);
        else
            glDrawElements ( GL_TRIANGLES, 3*shadowData.I_sides, GL_UNSIGNED_SHORT, 0);
    }
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
}
void RenderShadowVolumeZFailVBO(xShadowData &shadowData, bool infiniteL)
{
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, shadowData.gpuShadowPointers.vertexB );
    glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, shadowData.gpuShadowPointers.indexB );
    /************************* RENDER FACES ****************************/
    glVertexPointer   (4, GL_FLOAT, sizeof(xVector4), 0);

    if (GLExtensions::Exists_EXT_StencilTwoSide)
    {
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
        glColor4f(1.f, 0.f, 0.f, 0.4f);

/*
        if (!infiniteL)
        {
            // Extruded quads
            glDrawElements ( GL_QUADS, 4*shadowData.I_sides, GL_UNSIGNED_SHORT, 0);
            // Front cap
            glDrawElements ( GL_TRIANGLES, 3*shadowData.I_fronts, GL_UNSIGNED_SHORT,
                (GLvoid*)(4*shadowData.I_sides*sizeof(xWORD)));
            // Back cap
            glDrawElements ( GL_TRIANGLES, 3*shadowData.I_backs, GL_UNSIGNED_SHORT,
                (GLvoid*)((4*shadowData.I_sides + 3*shadowData.I_fronts)*sizeof(xWORD)));
        }
        else
        {
            // Extruded tris
            glDrawElements ( GL_TRIANGLES, 3*shadowData.I_sides, GL_UNSIGNED_SHORT, 0);
            // Front cap
            glDrawElements ( GL_TRIANGLES, 3*shadowData.I_fronts, GL_UNSIGNED_SHORT,
                (GLvoid*)(3*shadowData.I_sides*sizeof(xWORD)));
        }
*/
        if (!infiniteL)
        {
            // Extruded quads
            glDrawElements ( GL_QUADS, 4*shadowData.I_sides, GL_UNSIGNED_SHORT, 0);
            // Front & Back cap
            glDrawElements ( GL_TRIANGLES, 3*(shadowData.I_fronts+shadowData.I_backs), GL_UNSIGNED_SHORT,
                (GLvoid*)(4*shadowData.I_sides*sizeof(xWORD)));
        }
        else// Extruded tris & Front cap
            glDrawElements ( GL_TRIANGLES, 3*(shadowData.I_sides+shadowData.I_fronts), GL_UNSIGNED_SHORT, 0);
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
            glDrawElements ( GL_QUADS, 4*shadowData.I_sides, GL_UNSIGNED_SHORT, 0);
            // Front & Back cap
            glDrawElements ( GL_TRIANGLES, 3*(shadowData.I_fronts+shadowData.I_backs), GL_UNSIGNED_SHORT,
                (GLvoid*)(4*shadowData.I_sides*sizeof(xWORD)));
        }
        else// Extruded tris & Front cap
            glDrawElements ( GL_TRIANGLES, 3*(shadowData.I_sides+shadowData.I_fronts), GL_UNSIGNED_SHORT, 0);

        glCullFace(GL_FRONT);
        if (GLExtensions::Exists_EXT_StencilWrap)
            glStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
        else
            glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
        glColor4f(0.f, 1.f, 0.f, 0.4f);

        if (!infiniteL)
        {
            // Extruded quads
            glDrawElements ( GL_QUADS, 4*shadowData.I_sides, GL_UNSIGNED_SHORT, 0);
            // Front & Back cap
            glDrawElements ( GL_TRIANGLES, 3*(shadowData.I_fronts+shadowData.I_backs), GL_UNSIGNED_SHORT,
                (GLvoid*)(4*shadowData.I_sides*sizeof(xWORD)));
        }
        else// Extruded tris & Front cap
            glDrawElements ( GL_TRIANGLES, 3*(shadowData.I_sides+shadowData.I_fronts), GL_UNSIGNED_SHORT, 0);
    }
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
}


void RenderShadowVolumeElemVBO (xElement *elem, xModelInstance &modelInstance, xLight &light, xFieldOfView &FOV)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderShadowVolumeElemVBO(selem, modelInstance, light, FOV);

    if (!elem->renderData.I_vertices) return;
    if (!elem->I_edges) return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];

    if (!light.elementReceivesLight((elem->MX_MeshToLocal * modelInstance.MX_LocalToWorld).preTransformP(instance.bsCenter), instance.bsRadius)) return;

    bool zPass = !ShadowVolume::ViewportMaybeShadowed(elem, instance, modelInstance.MX_LocalToWorld, FOV, light);
    bool infiniteL = light.type == xLight_INFINITE;

    bool changed = false;
    xShadowData &shadowData = instance.GetShadowData(light, zPass ? xShadowData::ZPASS_ONLY : xShadowData::ZFAIL_PASS);
    if (!shadowData.L_indices)
    {
        xMatrix  mtxWorldToObject = (elem->MX_MeshToLocal * modelInstance.MX_LocalToWorld).invert();
        xVector3 lightPos;
        if (infiniteL)
            lightPos = mtxWorldToObject.preTransformV(light.position);
        else
            lightPos = mtxWorldToObject.preTransformP(light.position);

        bool *facingFlag = NULL;
        bool  useBackCapOptimization
            = (lightPos - instance.bsCenter).lengthSqr() > instance.bsRadius*instance.bsRadius;

        if (!shadowData.L_vertices)
            shadowData.L_vertices = new xVector4[(infiniteL) ? instance.I_vertices + 1 : instance.I_vertices << 1];
        memcpy (shadowData.L_vertices, instance.L_vertices, instance.I_vertices*sizeof(xVector4));
        ShadowVolume::ExtrudePoints(elem, infiniteL, lightPos, shadowData);
        ShadowVolume::GetBackFaces (elem, instance, infiniteL, shadowData, facingFlag);
        ShadowVolume::GetSilhouette(elem, infiniteL, useBackCapOptimization, facingFlag, shadowData);
        delete[] facingFlag;

        changed = true;
    }
    if (!shadowData.gpuShadowPointers.indexB)
    {
        GLuint p;
        glGenBuffersARB(1, &p); shadowData.gpuShadowPointers.vertexB = p;
        glBindBufferARB( GL_ARRAY_BUFFER_ARB, shadowData.gpuShadowPointers.vertexB );
        glBufferDataARB( GL_ARRAY_BUFFER_ARB,
            sizeof(xVector4)*((infiniteL) ? instance.I_vertices + 1 : instance.I_vertices << 1),
            shadowData.L_vertices, GL_DYNAMIC_DRAW_ARB);
        glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );

        glGenBuffersARB(1, &p); shadowData.gpuShadowPointers.indexB = p;
        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, shadowData.gpuShadowPointers.indexB );
        glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(xWORD)*shadowData.I_indices, shadowData.L_indices, GL_DYNAMIC_DRAW_ARB);
        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    }
    else
    if (changed)
    {
        glBindBufferARB( GL_ARRAY_BUFFER_ARB, shadowData.gpuShadowPointers.vertexB );
        glBufferSubDataARB( GL_ARRAY_BUFFER_ARB, sizeof(xVector4)*instance.I_vertices,
            sizeof(xVector4)*((infiniteL) ? 1 : instance.I_vertices),
            shadowData.L_vertices+instance.I_vertices);
        glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );

        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, shadowData.gpuShadowPointers.indexB );
        glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(xWORD)*shadowData.I_indices, shadowData.L_indices, GL_DYNAMIC_DRAW_ARB);
        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    }

    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.x0);

    if (zPass)
        RenderShadowVolumeZPassVBO(shadowData, infiniteL);
    else
        RenderShadowVolumeZFailVBO(shadowData, infiniteL);

    glPopMatrix();
}
/*****************************     COMMON     *********************************/
void RendererGL :: RenderShadowVolume(xModel &model, xModelInstance &instance, xLight &light, xFieldOfView &FOV)
{
    glPushMatrix();
    glMultMatrixf(&instance.MX_LocalToWorld.x0);

    glEnableClientState(GL_VERTEX_ARRAY);

    for (xElement *elem = model.L_kids; elem; elem = elem->Next)
        //if (UseVBO)
        //    RenderShadowVolumeElemVBO(elem, instance, light, FOV);
        //else
            RenderShadowVolumeElem(elem, instance, light, FOV);

    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}
