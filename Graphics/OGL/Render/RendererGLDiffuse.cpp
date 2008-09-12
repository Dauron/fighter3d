#include "RendererGL.h"
#include "../AnimSkeletal.h"

using namespace Graphics::OGL;

/********************************* diffuse model ************************************/
void RenderElementDiffuseLST(bool transparent, const Math::Cameras::FieldOfView &FOV,
                             const xLight &light, xElement *elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderElementDiffuseLST(transparent, FOV, light, selem, modelInstance);

    if (!elem->renderData.I_vertices
        || (transparent && !elem->FL_transparent)
        || (!transparent && !elem->FL_opaque)) return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];
    xMatrix MX_MeshToWorld = elem->MX_MeshToLocal * modelInstance.MX_LocalToWorld;
    instance.Transform(MX_MeshToWorld);
    if ( !light.elementReceivesLight(*instance.bSphere_T) ) return;
    if ( !light.elementReceivesDiffuseLight(FOV, instance.bBox_T->P_corners) )
    {
        ++Performance.CulledDiffuseElements;
        return;
    }
    if ( !FOV.CheckSphere(*instance.bSphere_T) || !FOV.CheckBox( *instance.bBox_T ) )
    {
        ++Performance.CulledElements;
        return;
    }

    /************************** PREPARE LST  ****************************/
    xDWORD &listID    = elem->FL_skeletized
        ? transparent ? instance.gpuMain.listIDTransp : instance.gpuMain.listID
        : transparent ? elem->renderData.gpuMain.listIDTransp : elem->renderData.gpuMain.listID;
    xDWORD &listIDTex = elem->FL_skeletized
        ? transparent ? instance.gpuMain.listIDTexTransp : instance.gpuMain.listIDTex
        : transparent ? elem->renderData.gpuMain.listIDTexTransp : elem->renderData.gpuMain.listIDTex;
    xGPUPointers::Mode &mode = elem->FL_skeletized ? instance.mode : elem->renderData.mode;
    bool textured = false;

    if (elem->FL_skeletized)
        Shader::EnableSkeleton(xState_On);

    if (!listID)
    {
        Shader::EnableTexturing(xState_Off);
        Shader::Start();

        mode = xGPUPointers::LIST;
        glNewList(listID = glGenLists(1), GL_COMPILE);

        if (elem->FL_skeletized) {
            g_AnimSkeletal.BeginAnimation();
            g_AnimSkeletal.SetBones(modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones,
                                modelInstance.P_bone_roots, modelInstance.P_bone_trans, elem, false);
            g_AnimSkeletal.SetElement(elem, &instance);
        }
        else
        {
            if (elem->FL_textured)
                glVertexPointer   (3, GL_FLOAT, sizeof(xVertexTex), elem->renderData.L_verticesT);
            else
                glVertexPointer   (3, GL_FLOAT, sizeof(xVertex), elem->renderData.L_vertices);
            /////////////////////////// LOAD NORMALS ///////////////////////////
            if (elem->renderData.L_normals)
            {
                glNormalPointer (GL_FLOAT, sizeof(xVector3), elem->renderData.L_normals);
                glEnableClientState(GL_NORMAL_ARRAY);
            }
        }

        xFaceList *faceL = elem->L_faceLists;
        xMaterial *m_currentMaterial = (xMaterial*)1;
        for(int i=elem->I_faceLists; i; --i, ++faceL)
        {
            if ((transparent && (!faceL->Material || faceL->Material->transparency == 0.f)) ||
                (!transparent && faceL->Material && faceL->Material->transparency > 0.f) )
                continue;
            if (elem->FL_textured && faceL->Material && faceL->Material->texture.htex)
            {
                textured = true;
                continue;
            }
            if (faceL->Material != m_currentMaterial)
                RendererGL::SetMaterial(elem->Color, m_currentMaterial = faceL->Material, false);
            glDrawElements(GL_TRIANGLES, 3*faceL->I_count, GL_UNSIGNED_SHORT, elem->renderData.L_faces+faceL->I_offset);
        }
        if (!textured && elem->renderData.L_normals) glDisableClientState(GL_NORMAL_ARRAY);
        if (!textured && elem->FL_skeletized)
            g_AnimSkeletal.EndAnimation();

        glEndList();
    }

    if (textured && elem->FL_textured && !listIDTex)
    {
        Shader::EnableTexturing(xState_On);
        Shader::Start();

        glNewList(listIDTex = glGenLists(1), GL_COMPILE);

        if (elem->FL_skeletized) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer (2, GL_FLOAT, sizeof(xVertexTexSkel), &(elem->renderData.L_verticesTS->tex.u));
            g_AnimSkeletal.SetBones(modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones,
                                modelInstance.P_bone_roots, modelInstance.P_bone_trans, elem, false);
        }
        else {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer (2, GL_FLOAT, sizeof(xVertexTex), &(elem->renderData.L_verticesT->tex.u));
        }

        xFaceList *faceL = elem->L_faceLists;
        xMaterial *m_currentMaterial = (xMaterial*)1;
        for(int i=elem->I_faceLists; i; --i, ++faceL)
        {
            if ((transparent && (!faceL->Material || faceL->Material->transparency == 0.f)) ||
                (!transparent && faceL->Material && faceL->Material->transparency > 0.f) )
                continue;
            if (!faceL->Material || !faceL->Material->texture.htex)
                continue;
            if (faceL->Material != m_currentMaterial)
                RendererGL::SetMaterial(elem->Color, m_currentMaterial = faceL->Material, false);
            glDrawElements(GL_TRIANGLES, 3*faceL->I_count, GL_UNSIGNED_SHORT, elem->renderData.L_faces+faceL->I_offset);

        }
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if (elem->renderData.L_normals) glDisableClientState(GL_NORMAL_ARRAY);
        if (elem->FL_skeletized)
            g_AnimSkeletal.EndAnimation();

        glEndList();
    }

    /************************* USE LST ****************************/
    if (listID)
    {
        glPushMatrix();
        {
            glMultMatrixf(&elem->MX_MeshToLocal.x0);
            Shader::EnableTexturing(xState_Off);
            Shader::Start();
            glCallList(listID);
            if (listIDTex)
            {
                Shader::EnableTexturing(xState_On);
                Shader::Start();
                glCallList(listIDTex);
            }
        }
        glPopMatrix();
    }

    Shader::EnableSkeleton(xState_Off);
}

void RenderElementDiffuseVBO(bool transparent, const Math::Cameras::FieldOfView &FOV,
                             const xLight &light, xElement *elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderElementDiffuseVBO(transparent, FOV, light, selem, modelInstance);

    if (!elem->renderData.I_vertices
        || (transparent && !elem->FL_transparent)
        || (!transparent && !elem->FL_opaque)) return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];
    xMatrix MX_MeshToWorld = elem->MX_MeshToLocal * modelInstance.MX_LocalToWorld;
    instance.Transform(MX_MeshToWorld);
    if ( !light.elementReceivesLight(*instance.bSphere_T) ) return;
    if ( !light.elementReceivesDiffuseLight(FOV, instance.bBox_T->P_corners) )
    {
        ++Performance.CulledDiffuseElements;
        return;
    }
    if ( !FOV.CheckSphere(*instance.bSphere_T) || !FOV.CheckBox( *instance.bBox_T ) )
    {
        ++Performance.CulledElements;
        return;
    }

    /************************* INIT VBO ****************************/
    if (elem->renderData.mode == xGPUPointers::NONE)
        RendererGL::InitVBO(elem);

    /************************* LOAD VERTICES ****************************/
    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.x0);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
    if (elem->FL_skeletized) {
        if (elem->FL_textured) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer (2, GL_FLOAT, sizeof(xVertexTexSkel), (void *)(7*sizeof(xFLOAT)));
        }
        Shader::EnableSkeleton(xState_On);
        Shader::Start();
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones,
                                modelInstance.P_bone_roots, modelInstance.P_bone_trans, elem, true);
        g_AnimSkeletal.SetElement(elem, &instance, true);
    }
    else
    {
        if (elem->FL_textured) {
            glVertexPointer   (3, GL_FLOAT, sizeof(xVertexTex), 0);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer (2, GL_FLOAT, sizeof(xVertexTex), (void *)(3*sizeof(xFLOAT)));
        }
        else
            glVertexPointer   (3, GL_FLOAT, sizeof(xVertex), 0);
        /************************* LOAD NORMALS ****************************/
        if (elem->renderData.L_normals) {
            glBindBufferARB ( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.normalB );
            glNormalPointer ( GL_FLOAT, sizeof(xVector3), 0 );
            glEnableClientState(GL_NORMAL_ARRAY);
            glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
        }
    }

    /************************* RENDER FACES ****************************/
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.indexB );
    xMaterial *m_currentMaterial = (xMaterial*)1;
    xFaceList *faceL = elem->L_faceLists;
    for(int i=elem->I_faceLists; i; --i, ++faceL)
    {
        if ((transparent && (!faceL->Material || faceL->Material->transparency == 0.f)) ||
            (!transparent && faceL->Material && faceL->Material->transparency > 0.f) )
            continue;
        if (faceL->Material != m_currentMaterial)
        {
            RendererGL::SetMaterial(elem->Color, m_currentMaterial = faceL->Material);
            if (elem->FL_skeletized)
                g_AnimSkeletal.SetBones(modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones,
                                modelInstance.P_bone_roots, modelInstance.P_bone_trans, elem, true);
        }
        glDrawElements(GL_TRIANGLES, 3*faceL->I_count, GL_UNSIGNED_SHORT, (void*)(faceL->I_offset*3*sizeof(xWORD)));
    }
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB ( GL_ARRAY_BUFFER_ARB, 0 );

    if (elem->FL_textured)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    if (elem->renderData.L_normals)
        glDisableClientState(GL_NORMAL_ARRAY);
    if (elem->FL_skeletized)
        g_AnimSkeletal.EndAnimation();
    Shader::EnableSkeleton(xState_Off);

    glPopMatrix();
}

void RendererGL :: RenderDiffuse(xModel &model, xModelInstance &instance, const xLight &light,
                              bool transparent, const Math::Cameras::FieldOfView &FOV)
{
    if ((transparent  && !model.FL_transparent) ||
        (!transparent && !model.FL_opaque)) return;

    InitTextures(model);
    glEnableClientState(GL_VERTEX_ARRAY);

    glPushMatrix();
    glMultMatrixf(&instance.MX_LocalToWorld.x0);

    for (xElement *elem = model.L_kids; elem; elem = elem->Next)
        // NOTE: MIX of display lists and VBO appears to be much slower than VBO only
        if (UseVBO)
            RenderElementDiffuseVBO(transparent, FOV, light, elem, instance);
        else
            RenderElementDiffuseLST(transparent, FOV, light, elem, instance);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
}
