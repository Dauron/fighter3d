#include "RendererGL.h"
#include "../AnimSkeletal.h"
#include "../../Textures/TextureMgr.h"

using namespace Graphics::OGL;

/********************************* common private ************************************/
void RendererGL :: InitVBO (xElement *elem)
{
    if (elem->renderData.mode == xGPUPointers::NONE)
    {
        elem->renderData.mode = xGPUPointers::VBO;
        int stride = elem->GetVertexStride();
        GLuint p;
        glGenBuffersARB(1, &p); elem->renderData.gpuMain.vertexB = p;
        glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
        glBufferDataARB( GL_ARRAY_BUFFER_ARB, stride*elem->renderData.I_vertices, elem->renderData.L_vertices, GL_STATIC_DRAW_ARB);

        glGenBuffersARB(1, &p); elem->renderData.gpuMain.indexB = p;
        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.indexB );
        glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(xWORD)*3*elem->I_faces, elem->renderData.L_faces, GL_STATIC_DRAW_ARB);
        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );

        if (elem->renderData.L_normals)
        {
            glGenBuffersARB(1, &p); elem->renderData.gpuMain.normalB = p;
            glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.normalB );
            glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(xVector3)*elem->renderData.I_vertices, elem->renderData.L_normals, GL_STATIC_DRAW_ARB);
        }

        if (elem->renderData.L_colors)
        {
            glGenBuffersARB(1, &p); elem->renderData.gpuMain.colorB = p;
            glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.colorB );
            glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(xBYTE3)*elem->renderData.I_vertices, elem->renderData.L_colors, GL_STATIC_DRAW_ARB);
        }

        glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
    }
}

void RendererGL :: InitTextures(xModel &model)
{
    if (model.FL_textures_loaded)
        return;

    for (xMaterial *mat = model.L_material; mat; mat = mat->Next)
        if (mat->texture.Name)
        {
            char tmp[74] = "Data/textures/";
            char *itr = mat->texture.Name;
            for (; *itr; ++itr) *itr = tolower(*itr);
            strcat(tmp, mat->texture.Name);
            mat->texture.htex = g_TextureMgr.GetTexture(tmp).GetHandle();
        }
        else
            mat->texture.htex = 0;
    model.FL_textures_loaded = true;
}

void RendererGL::SetMaterial(xMaterial *mat, bool toggleShader)
{
    if (mat)
    {
        glDisable(GL_COLOR_MATERIAL);

        float shininess = pow(2, mat->shininess_gloss);
        if (shininess > 128.0)
            shininess = 128.0;
        glMaterialf(GL_FRONT, GL_SHININESS, shininess);

        if (mat->texture.htex) // if texture is loaded
        {
            HTexture htext;
            htext.SetHandle(mat->texture.htex);
            g_TextureMgr.BindTexture(htext);
            Shader::EnableTexturing(xState_On);
        }
        else
        {
            Shader::EnableTexturing(xState_Off);
            //float specular[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            //glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        }
        if (Config::EnableLighting)
        {
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            if (mat->transparency > 0.f)
            {
                xColor t_mat; t_mat.init(mat->ambient.vector3, 1.f-mat->transparency);
                glMaterialfv(GL_FRONT, GL_AMBIENT, t_mat.rgba);
                t_mat.vector3 = mat->diffuse.vector3;
                glMaterialfv(GL_FRONT, GL_DIFFUSE, t_mat.rgba);
                t_mat.vector3 = mat->specular.vector3 * mat->shininess_level;
                glMaterialfv(GL_FRONT, GL_SPECULAR, t_mat.rgba);
            }
            else
            {
                glMaterialfv(GL_FRONT, GL_AMBIENT, mat->ambient.rgba);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, mat->diffuse.rgba);
                xColor spec = mat->specular * mat->shininess_level;
                glMaterialfv(GL_FRONT, GL_SPECULAR, spec.rgba);
            }
        }
        else
        {
            if (mat->transparency > 0.f)
            {
                xColor t_mat; t_mat.init(mat->diffuse.vector3, 1.f-mat->transparency);
                glColor4fv(t_mat.rgba);
            }
            else
                glColor4fv(mat->diffuse.rgba);

        }
        if (mat->two_sided)
            glDisable(GL_CULL_FACE);
        else
            glEnable(GL_CULL_FACE);
    }
    else
    {
        glEnable(GL_CULL_FACE);

        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);
        /*
        float ambient[4]  = {0.0f, 0.0f, 0.0f, 1.0f};
        float diffuse[4]  = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT, GL_AMBIENT,  ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  diffuse);
        */
        float specular[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 2.0f);
        Shader::EnableTexturing(xState_Off);
    }
    if (toggleShader)
        Shader::Start();
}
/*
void SetLight(const xLight &light, bool t_Ambient, bool t_Diffuse, bool t_Specular)
{
    float light_off[4] = { 0.f, 0.f, 0.f, 0.f };
    xColor ambient; ambient = light.color * light.softness;
    xColor diffuse; diffuse = light.color - ambient;

    glLightfv(GL_LIGHT0, GL_AMBIENT, t_Ambient ? ambient.col : light_off);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  t_Diffuse ? diffuse.col : light_off); // direct light
    glLightfv(GL_LIGHT0, GL_SPECULAR, t_Specular ? diffuse.col : light_off); // light on mirrors/metal

    Shader::SetLightType(light.type, t_Ambient, t_Diffuse, t_Specular);
}
*/
/********************************* faces **************************************/
void RenderElementFacesVBO(
                            xWORD                 selElementId,
                            std::vector<xDWORD> * facesToRender,
                            xElement            * elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderElementFacesVBO(selElementId, facesToRender, selem, modelInstance);

    if (!elem->renderData.I_vertices || elem->ID != selElementId)
        return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];
    /************************* INIT VBO ****************************/
    if (elem->renderData.mode == xGPUPointers::NONE)
        RendererGL::InitVBO(elem);

    /************************* LOAD VERTICES ****************************/
    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.matrix[0][0]);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
    int stride;
    if (elem->FL_skeletized) {
        stride = elem->FL_textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones,
                                modelInstance.P_bone_roots, modelInstance.P_bone_trans, elem, true);
        g_AnimSkeletal.SetElement(elem, &instance, true);
    }
    else {
        stride = elem->FL_textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer (3, GL_FLOAT, stride, 0);
    }
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );

    /************************* RENDER ****************************/
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.indexB );

    std::vector<xDWORD>::iterator iter = facesToRender->begin();
    for (; iter != facesToRender->end(); ++iter)
    {
        float r = (rand() % 9) / 10.f;
        float g = (rand() % 9) / 10.f;
        float b = (rand() % 9) / 10.f;
        glColor3f(0.1f+r, 0.1f+g, 0.1f+b);
        glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (GLvoid*) (*iter - (off_t)elem->L_faces) );
    }
    /*
    if (elem->collisionData.hierarchyP)
        for (int i=0; i<elem->collisionData.hierarchyC; ++i)
        {
            float r = (rand() % 9) / 10.f;
            float g = (rand() % 9) / 10.f;
            float b = (rand() % 9) / 10.f;
            glColor3f(0.1f+r, 0.1f+g, 0.1f+b);
            xFace **iterF = elem->collisionData.hierarchyP[i].L_faces;
            for (int j = elem->collisionData.hierarchyP[i].I_faces; j; --j, ++iterF)
                glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (GLvoid*) ((xDWORD)(*iterF) - (xDWORD)elem->L_faces) );
        }
    */
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );

    if (elem->FL_skeletized)
        g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

void RenderElementFacesLST(
                            xWORD                 selElementId,
                            std::vector<xDWORD> * facesToRender,
                            xElement            * elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderElementFacesLST(selElementId, facesToRender, selem, modelInstance);

    if (!elem->renderData.I_vertices || elem->ID != selElementId)
        return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];

    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.matrix[0][0]);

    int stride;
    if (elem->FL_skeletized) {
        stride = elem->FL_textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones,
                                modelInstance.P_bone_roots, modelInstance.P_bone_trans, elem, false);
        g_AnimSkeletal.SetElement(elem, &instance);
    }
    else {
        stride = elem->FL_textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer (3, GL_FLOAT, stride, elem->renderData.L_vertices);
    }

    std::vector<xDWORD>::iterator iter = facesToRender->begin();
    for (; iter != facesToRender->end(); ++iter)
    {
        float r = (rand() % 9) / 10.f;
        float g = (rand() % 9) / 10.f;
        float b = (rand() % 9) / 10.f;
        glColor3f(0.1f+r, 0.1f+g, 0.1f+b);
        glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (GLvoid *)(off_t)*iter );
    }
    /*
    if (elem->collisionData.hierarchyP)
        for (int i=0; i<elem->collisionData.hierarchyC; ++i)
        {
            float r = (rand() % 9) / 10.f;
            float g = (rand() % 9) / 10.f;
            float b = (rand() % 9) / 10.f;
            glColor3f(0.1f+r, 0.1f+g, 0.1f+b);
            xFace **iterF = elem->collisionData.hierarchyP[i].L_faces;
            for (int j = elem->collisionData.hierarchyP[i].I_faces; j; --j, ++iterF)
                glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, *iterF);
        }
    */

    if (elem->FL_skeletized)
        g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

void RendererGL :: RenderFaces( xModel &model, xModelInstance &instance,
                               xWORD                 selectedElement,
                               std::vector<xDWORD> * facesToRender)
{
    glEnableClientState(GL_VERTEX_ARRAY);

    glColor4f( 0.8f, 0.8f, 0.f, 1.f );
    Shader::EnableTexturing(xState_Disable);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    for (xElement *elem = model.L_kids; elem; elem = elem->Next)
        if (UseVBO)
            RenderElementFacesVBO(selectedElement, facesToRender, elem, instance);
        else
            RenderElementFacesLST(selectedElement, facesToRender, elem, instance);

    Shader::EnableTexturing(xState_Enable);
    glDisable(GL_COLOR_MATERIAL);
    glDisableClientState(GL_VERTEX_ARRAY);
}

/********************************* vertices && element selections ************************************/
void RenderElementVerticesVBO(
                            Renderer::SelectionMode selectionMode,
                            xWORD                   selElementId,
                            std::vector<xDWORD>   * selectedVertices,
                            xElement              * elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderElementVerticesVBO(selectionMode, selElementId, selectedVertices, selem, modelInstance);

    if (!elem->renderData.I_vertices || (selectionMode != Renderer::smElement && selectionMode != Renderer::smModel && elem->ID != selElementId))
        return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];
    /************************* INIT VBO ****************************/
    if (elem->renderData.mode == xGPUPointers::NONE)
        RendererGL::InitVBO(elem);

    if (selectionMode == Renderer::smElement)
        glLoadName(elem->ID);
    /************************* LOAD VERTICES ****************************/
    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.matrix[0][0]);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
    int stride;
    if (elem->FL_skeletized) {
        stride = elem->FL_textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones,
                                modelInstance.P_bone_roots, modelInstance.P_bone_trans, elem, true);
        g_AnimSkeletal.SetElement(elem, &instance, true);
    }
    else {
        stride = elem->FL_textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer (3, GL_FLOAT, stride, 0);
    }
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );

    /************************* RENDER ****************************/
    if (selectionMode == Renderer::smVertex)
        for (GLuint i=0; i < elem->renderData.I_vertices; ++i)
        {
            glLoadName(i);
            glDrawArrays(GL_POINTS, i, 1);
        }
    else
    if (selectionMode == Renderer::smElement || selectionMode == Renderer::smModel) {
        glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.indexB );
        glDrawElements (GL_TRIANGLES, 3*elem->I_faces, GL_UNSIGNED_SHORT, 0);
        glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    }
    else { //if (selectionMode == smNone) {
        xColor cNotSkinned; cNotSkinned.init(0.8f, 0.8f, 0.f, 1.f);
        xColor cSkinned;    cSkinned.init(0.f, 0.8f, 0.8f, 1.f);
        xColor cSelected;   cSelected.init(1.f, 0.f, 0.f, 1.f);

        xColor *colors = new xColor[elem->renderData.I_vertices];
        xColor *citer  = colors;
        xBYTE  *ptr = (xBYTE*) elem->renderData.L_vertices;

        for (GLuint i=0; i < elem->renderData.I_vertices; ++i, ++citer, ptr += stride)
        {
            if ( !elem->FL_skeletized || ((xVertexSkel*)ptr)->b0 == 0.1f )
                memcpy (citer, &cNotSkinned, sizeof(xColor));
            else
                memcpy (citer, &cSkinned, sizeof(xColor));
        }
        if (selectedVertices != NULL) {
            std::vector<xDWORD>::iterator iter = selectedVertices->begin();
            for (; iter != selectedVertices->end(); ++iter)
                memcpy (colors + *iter, &cSelected, sizeof(xColor));
        }

        glEnableClientState (GL_COLOR_ARRAY);
        glColorPointer      (4, GL_FLOAT, sizeof(xColor), colors);
        glDrawArrays        (GL_POINTS, 0, elem->renderData.I_vertices);
        glDisableClientState(GL_COLOR_ARRAY);
        delete[] colors;
    }

    if (elem->FL_skeletized)
        g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

void RenderElementVerticesLST(
                            Renderer::SelectionMode selectionMode,
                            xWORD                   selElementId,
                            std::vector<xDWORD>   * selectedVertices,
                            xElement              * elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderElementVerticesLST(selectionMode, selElementId, selectedVertices, selem, modelInstance);

    if (!elem->renderData.I_vertices || (selectionMode != Renderer::smElement && selectionMode != Renderer::smModel && elem->ID != selElementId))
        return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];

    if (selectionMode == Renderer::smElement)
        glLoadName(elem->ID);
    glPushMatrix();
    glMultMatrixf(&elem->MX_MeshToLocal.matrix[0][0]);

    int stride;
    if (elem->FL_skeletized) {
        stride = elem->FL_textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(modelInstance.I_bones, modelInstance.MX_bones, modelInstance.QT_bones,
                                modelInstance.P_bone_roots, modelInstance.P_bone_trans, elem, false);
        g_AnimSkeletal.SetElement(elem, &instance);
    }
    else {
        stride = elem->FL_textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer (3, GL_FLOAT, stride, elem->renderData.L_vertices);
    }

    if (selectionMode == Renderer::smVertex)
        for (GLuint i=0; i < elem->renderData.I_vertices; ++i)
        {
            glLoadName(i);
            glDrawArrays(GL_POINTS, i, 1);
        }
    else
    if (selectionMode == Renderer::smElement || selectionMode == Renderer::smModel)
        glDrawElements (GL_TRIANGLES, 3*elem->I_faces, GL_UNSIGNED_SHORT, elem->renderData.L_faces);
    else { // if (selectionMode == smNone) {
        xColor cNotSkinned; cNotSkinned.init(0.8f, 0.8f, 0.f, 1.f);
        xColor cSkinned;    cSkinned.init(0.f, 0.8f, 0.8f, 1.f);
        xColor cSelected;   cSelected.init(1.f, 0.f, 0.f, 1.f);

        xColor *colors = new xColor[elem->renderData.I_vertices];
        xColor *citer  = colors;
        xBYTE  *ptr = (xBYTE*) elem->renderData.L_vertices;

        for (GLuint i=0; i < elem->renderData.I_vertices; ++i, ++citer, ptr += stride)
        {
            if ( !elem->FL_skeletized || ((xVertexSkel*)ptr)->b0 == 0.1f )
                memcpy (citer, &cNotSkinned, sizeof(xColor));
            else
                memcpy (citer, &cSkinned, sizeof(xColor));
        }
        if (selectedVertices != NULL) {
            std::vector<xDWORD>::iterator iter = selectedVertices->begin();
            for (; iter != selectedVertices->end(); ++iter)
                memcpy (colors + *iter, &cSelected, sizeof(xColor));
        }

        glEnableClientState (GL_COLOR_ARRAY);
        glColorPointer      (4, GL_FLOAT, sizeof(xColor), colors);
        glDrawArrays        (GL_POINTS, 0, elem->renderData.I_vertices);
        glDisableClientState(GL_COLOR_ARRAY);
        delete[] colors;
    }

    if (elem->FL_skeletized)
        g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

void RendererGL :: RenderVertices( xModel &model, xModelInstance &instance,
                                  SelectionMode         selectionMode,
                                  xWORD                 selectedElement,
                                  std::vector<xDWORD> * selectedVertices)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    GLint prevMode[2];
    GLfloat prevSize;
    glGetIntegerv(GL_POLYGON_MODE, prevMode);
    glGetFloatv(GL_POINT_SIZE, &prevSize);
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    glPointSize(5);

    glPushMatrix();
    glMultMatrixf(&instance.MX_LocalToWorld.x0);

    glColor4f( 0.8f, 0.8f, 0.f, 1.f );
    Shader::EnableTexturing(xState_Disable);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    if (selectionMode != smNone) g_AnimSkeletal.ForceSoftware(true);

    for (xElement *elem = model.L_kids; elem; elem = elem->Next)
        if (UseVBO)
            RenderElementVerticesVBO(selectionMode, selectedElement, selectedVertices, elem, instance);
        else
            RenderElementVerticesLST(selectionMode, selectedElement, selectedVertices, elem, instance);

    if (selectionMode != smNone) g_AnimSkeletal.ForceSoftware(false);

    glDisable(GL_COLOR_MATERIAL);
    Shader::EnableTexturing(xState_Enable);
    glPopMatrix();

    glPolygonMode(GL_FRONT, prevMode[0]);
    glPolygonMode(GL_BACK,  prevMode[1]);
    glPointSize(prevSize);
    glDisableClientState(GL_VERTEX_ARRAY);
}


/********************************* model ************************************/
void RenderModelLST(bool transparent, const Math::Cameras::FieldOfView &FOV,
                    xElement *elem, xModelInstance &modelInstance, bool UseList)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderModelLST(transparent, FOV, selem, modelInstance, UseList);

    if (!elem->renderData.I_vertices
        || (transparent && !elem->FL_transparent)
        || (!transparent && !elem->FL_opaque)) return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];
    xMatrix MX_MeshToWorld = elem->MX_MeshToLocal * modelInstance.MX_LocalToWorld;
    instance.Transform(MX_MeshToWorld);
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

        RendererGL::SetColors(elem, false);

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
                RendererGL::SetMaterial(m_currentMaterial = faceL->Material, false);
            glDrawElements(GL_TRIANGLES, 3*faceL->I_count, GL_UNSIGNED_SHORT, elem->renderData.L_faces+faceL->I_offset);
        }
        if (!textured && elem->renderData.L_normals) glDisableClientState(GL_NORMAL_ARRAY);
        if (!textured && elem->renderData.L_colors) glDisable(GL_COLOR_ARRAY);
        if (!textured && elem->FL_skeletized)
            g_AnimSkeletal.EndAnimation();

        glEndList();
    }

    if (textured && elem->FL_textured && !listIDTex)
    {
        glNewList(listIDTex = glGenLists(1), GL_COMPILE);

        Shader::EnableTexturing(xState_On);
        Shader::Start();

        RendererGL::SetColors(elem, false);

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
                RendererGL::SetMaterial(m_currentMaterial = faceL->Material, false);
            glDrawElements(GL_TRIANGLES, 3*faceL->I_count, GL_UNSIGNED_SHORT, elem->renderData.L_faces+faceL->I_offset);
        }
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if (elem->renderData.L_normals) glDisableClientState(GL_NORMAL_ARRAY);
        if (elem->renderData.L_colors) glDisable(GL_COLOR_ARRAY);
        if (elem->FL_skeletized)
            g_AnimSkeletal.EndAnimation();

        glEndList();
    }

    /************************* USE LST ****************************/
    if (UseList && listID)
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

void RenderModelVBO(bool transparent, const Math::Cameras::FieldOfView &FOV,
                    xElement *elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem->L_kids; selem; selem = selem->Next)
        RenderModelVBO(transparent, FOV, selem, modelInstance);

    if (!elem->renderData.I_vertices
        || (transparent && !elem->FL_transparent)
        || (!transparent && !elem->FL_opaque)) return;

    xElementInstance &instance = modelInstance.L_elements[elem->ID];
    xMatrix MX_MeshToWorld = elem->MX_MeshToLocal * modelInstance.MX_LocalToWorld;
    instance.Transform(MX_MeshToWorld);
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
    glMultMatrixf(&elem->MX_MeshToLocal.matrix[0][0]);

    RendererGL::SetColors(elem, true);

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
        if (Shader::NeedNormals() && elem->renderData.L_normals) {
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
            RendererGL::SetMaterial(m_currentMaterial = faceL->Material);
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
    if (elem->renderData.L_normals && Shader::NeedNormals())
        glDisableClientState(GL_NORMAL_ARRAY);
    if (elem->renderData.L_colors) glDisable(GL_COLOR_ARRAY);
    if (elem->FL_skeletized)
        g_AnimSkeletal.EndAnimation();
    Shader::EnableSkeleton(xState_Off);

    glPopMatrix();
}

void RendererGL :: RenderModel(xModel &model, xModelInstance &instance,
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
            RenderModelVBO(transparent, FOV, elem, instance);
        else
            RenderModelLST(transparent, FOV, elem, instance, UseList);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
}

