#include "xRenderGL.h"
#include "../GLAnimSkeletal.h"
#include "../Textures/TextureMgr.h"

/********************************* common private ************************************/
void xRenderGL :: InitVBO (xElement *elem)
{
    if (elem->renderData.mode == xGPURender::NONE)
    {
        elem->renderData.mode = xGPURender::VBO;
        int stride = (elem->skeletized && elem->textured) ? sizeof(xVertexTexSkel)
            : (elem->skeletized) ? sizeof(xVertexSkel)
            : (elem->textured) ? sizeof(xVertexTex) : sizeof(xVertex);
        GLuint p;
        glGenBuffersARB(1, &p); elem->renderData.gpuMain.vertexB = p;
        glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
        glBufferDataARB( GL_ARRAY_BUFFER_ARB, stride*elem->renderData.verticesC, elem->renderData.verticesP, GL_STATIC_DRAW_ARB);

        glGenBuffersARB(1, &p); elem->renderData.gpuMain.indexB = p;
        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.indexB );
        glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(xWORD)*3*elem->facesC, elem->renderData.facesP, GL_STATIC_DRAW_ARB);
        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );

        if (elem->renderData.normalP)
        {
            glGenBuffersARB(1, &p); elem->renderData.gpuMain.normalB = p;
            glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.normalB );
            glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(xVector3)*elem->renderData.verticesC, elem->renderData.normalP, GL_STATIC_DRAW_ARB);
        }
        glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
    }
}

void xRenderGL :: InitTextures(xModel &model)
{
    if (!model.texturesInited)
    for (xMaterial *mat = model.materialP; mat; mat = mat->nextP)
        if (mat->texture.name)
        {
            char tmp[74] = "Data/textures/";
            char *itr = mat->texture.name;
            for (; *itr; ++itr) *itr = tolower(*itr);
            strcat(tmp, mat->texture.name);
            mat->texture.htex = g_TextureMgr.GetTexture(tmp).GetHandle();
        }
        else
            mat->texture.htex = 0;
    model.texturesInited = true;
}

void xRenderGL::SetMaterial(xColor color, xMaterial *mat, bool toggleShader)
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
            GLShader::EnableTexturing(xState_On);
        }
        else
        {
            GLShader::EnableTexturing(xState_Off);
            //float specular[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            //glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        }
        if (Config::EnableLighting)
        {
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            if (mat->transparency > 0.f)
            {   
                xColor t_mat; t_mat.init(mat->ambient.vector3, 1.f-mat->transparency);
                glMaterialfv(GL_FRONT, GL_AMBIENT, t_mat.col);
                t_mat.vector3 = mat->diffuse.vector3;
                glMaterialfv(GL_FRONT, GL_DIFFUSE, t_mat.col);
                t_mat.vector3 = mat->specular.vector3 * mat->shininess_level;
                glMaterialfv(GL_FRONT, GL_SPECULAR, t_mat.col);
            }
            else
            {
                glMaterialfv(GL_FRONT, GL_AMBIENT, mat->ambient.col);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, mat->diffuse.col);
                xColor spec = mat->specular * mat->shininess_level;
                glMaterialfv(GL_FRONT, GL_SPECULAR, spec.col);
            }
        }
        else
        {
            if (mat->transparency > 0.f)
            {
                xColor t_mat; t_mat.init(mat->diffuse.vector3, 1.f-mat->transparency);
                glColor4fv(t_mat.col);
            }
            else
                glColor4fv(mat->diffuse.col);

        }
        if (mat->two_sided)
            glDisable(GL_CULL_FACE);
        else
            glEnable(GL_CULL_FACE);
    }
    else
    {
        glEnable(GL_CULL_FACE);
        glColor4fv(color.col);
        
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
        GLShader::EnableTexturing(xState_Off);
    }
    if (toggleShader)
        GLShader::Start();
}

void SetLight(const xLight &light, bool t_Ambient, bool t_Diffuse, bool t_Specular)
{
    float light_off[4] = { 0.f, 0.f, 0.f, 0.f };
    xColor ambient; ambient = light.color * light.softness;
    xColor diffuse; diffuse = light.color - ambient;

    glLightfv(GL_LIGHT0, GL_AMBIENT, t_Ambient ? ambient.col : light_off);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  t_Diffuse ? diffuse.col : light_off); // direct light
    glLightfv(GL_LIGHT0, GL_SPECULAR, t_Specular ? diffuse.col : light_off); // light on mirrors/metal
    
    GLShader::SetLightType(light.type, t_Ambient, t_Diffuse, t_Specular);
}
/********************************* faces **************************************/
void RenderElementFacesVBO(
                            xWORD                 selElementId,
                            std::vector<xDWORD> * facesToRender,
                            xElement            * elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderElementFacesVBO(selElementId, facesToRender, selem, modelInstance);

    if (!elem->renderData.verticesC || elem->id != selElementId)
        return;

    xElementInstance &instance = modelInstance.elementInstanceP[elem->id];
    /************************* INIT VBO ****************************/
    if (elem->renderData.mode == xGPURender::NONE)
        xRenderGL::InitVBO(elem);

    /************************* LOAD VERTICES ****************************/
    glPushMatrix();
    glMultMatrixf(&elem->matrix.matrix[0][0]);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
    int stride;
    if (elem->skeletized) {
        stride = elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(modelInstance.bonesC, modelInstance.bonesM, modelInstance.bonesQ, elem, true);
        g_AnimSkeletal.SetElement(elem, &instance, true);
    }
    else {
        stride = elem->textured ? sizeof(xVertexTex) : sizeof(xVertex);
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
        glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (GLvoid*) (*iter - (xDWORD)elem->facesP) );
    }
    /*
    if (elem->collisionData.hierarchyP)
        for (int i=0; i<elem->collisionData.hierarchyC; ++i)
        {
            float r = (rand() % 9) / 10.f;
            float g = (rand() % 9) / 10.f;
            float b = (rand() % 9) / 10.f;
            glColor3f(0.1f+r, 0.1f+g, 0.1f+b);
            xFace **iterF = elem->collisionData.hierarchyP[i].facesP;
            for (int j = elem->collisionData.hierarchyP[i].facesC; j; --j, ++iterF)
                glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (GLvoid*) ((xDWORD)(*iterF) - (xDWORD)elem->facesP) );
        }
    */
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );

    if (elem->skeletized)
        g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

void RenderElementFacesLST(
                            xWORD                 selElementId,
                            std::vector<xDWORD> * facesToRender,
                            xElement            * elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderElementFacesLST(selElementId, facesToRender, selem, modelInstance);

    if (!elem->renderData.verticesC || elem->id != selElementId)
        return;

    xElementInstance &instance = modelInstance.elementInstanceP[elem->id];
    
    glPushMatrix();
    glMultMatrixf(&elem->matrix.matrix[0][0]);

    int stride;
    if (elem->skeletized) {
        stride = elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(modelInstance.bonesC, modelInstance.bonesM, modelInstance.bonesQ, elem, false);
        g_AnimSkeletal.SetElement(elem, &instance);
    }
    else {
        stride = elem->textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer (3, GL_FLOAT, stride, elem->renderData.verticesP);
    }

    std::vector<xDWORD>::iterator iter = facesToRender->begin();
    for (; iter != facesToRender->end(); ++iter)
    {
        float r = (rand() % 9) / 10.f;
        float g = (rand() % 9) / 10.f;
        float b = (rand() % 9) / 10.f;
        glColor3f(0.1f+r, 0.1f+g, 0.1f+b);
        glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (GLvoid *) *iter );
    }
    /*
    if (elem->collisionData.hierarchyP)
        for (int i=0; i<elem->collisionData.hierarchyC; ++i)
        {
            float r = (rand() % 9) / 10.f;
            float g = (rand() % 9) / 10.f;
            float b = (rand() % 9) / 10.f;
            glColor3f(0.1f+r, 0.1f+g, 0.1f+b);
            xFace **iterF = elem->collisionData.hierarchyP[i].facesP;
            for (int j = elem->collisionData.hierarchyP[i].facesC; j; --j, ++iterF)
                glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, *iterF);
        }
    */

    if (elem->skeletized)
        g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

void xRenderGL :: RenderFaces( xModel &model, xModelInstance &instance,
                               xWORD                 selectedElement,
                               std::vector<xDWORD> * facesToRender)
{
    glEnableClientState(GL_VERTEX_ARRAY);

    glColor4f( 0.8f, 0.8f, 0.f, 1.f );
    GLShader::EnableTexturing(xState_Disable);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    for (xElement *elem = model.kidsP; elem; elem = elem->nextP)
        if (UseVBO)
            RenderElementFacesVBO(selectedElement, facesToRender, elem, instance);
        else
            RenderElementFacesLST(selectedElement, facesToRender, elem, instance);

    GLShader::EnableTexturing(xState_Enable);
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
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderElementVerticesVBO(selectionMode, selElementId, selectedVertices, selem, modelInstance);

    if (!elem->renderData.verticesC || (selectionMode != Renderer::smElement && selectionMode != Renderer::smModel && elem->id != selElementId))
        return;

    xElementInstance &instance = modelInstance.elementInstanceP[elem->id];
    /************************* INIT VBO ****************************/
    if (elem->renderData.mode == xGPURender::NONE)
        xRenderGL::InitVBO(elem);

    if (selectionMode == Renderer::smElement)
        glLoadName(elem->id);
    /************************* LOAD VERTICES ****************************/
    glPushMatrix();
    glMultMatrixf(&elem->matrix.matrix[0][0]);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
    int stride;
    if (elem->skeletized) {
        stride = elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(modelInstance.bonesC, modelInstance.bonesM, modelInstance.bonesQ, elem, true);
        g_AnimSkeletal.SetElement(elem, &instance, true);
    }
    else {
        stride = elem->textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer (3, GL_FLOAT, stride, 0);
    }
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );

    /************************* RENDER ****************************/
    if (selectionMode == Renderer::smVertex)
        for (GLuint i=0; i < elem->renderData.verticesC; ++i)
        {
            glLoadName(i);
            glDrawArrays(GL_POINTS, i, 1);
        }
    else
    if (selectionMode == Renderer::smElement || selectionMode == Renderer::smModel) {
        glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.indexB );
        glDrawElements (GL_TRIANGLES, 3*elem->facesC, GL_UNSIGNED_SHORT, 0);
        glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    }
    else { //if (selectionMode == smNone) {
        xColor cNotSkinned; cNotSkinned.init(0.8f, 0.8f, 0.f, 1.f);
        xColor cSkinned;    cSkinned.init(0.f, 0.8f, 0.8f, 1.f);
        xColor cSelected;   cSelected.init(1.f, 0.f, 0.f, 1.f);

        xColor *colors = new xColor[elem->renderData.verticesC];
        xColor *citer  = colors;
        xBYTE  *ptr = (xBYTE*) elem->renderData.verticesP;

        for (GLuint i=0; i < elem->renderData.verticesC; ++i, ++citer, ptr += stride)
        {
            if ( !elem->skeletized || ((xVertexSkel*)ptr)->b0 == 0.1f )
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
        glDrawArrays        (GL_POINTS, 0, elem->renderData.verticesC);
        glDisableClientState(GL_COLOR_ARRAY);
        delete[] colors;
    }

    if (elem->skeletized)
        g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

void RenderElementVerticesLST(
                            Renderer::SelectionMode selectionMode,
                            xWORD                   selElementId,
                            std::vector<xDWORD>   * selectedVertices,
                            xElement              * elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderElementVerticesLST(selectionMode, selElementId, selectedVertices, selem, modelInstance);

    if (!elem->renderData.verticesC || (selectionMode != Renderer::smElement && selectionMode != Renderer::smModel && elem->id != selElementId))
        return;

    xElementInstance &instance = modelInstance.elementInstanceP[elem->id];
    
    if (selectionMode == Renderer::smElement)
        glLoadName(elem->id);
    glPushMatrix();
    glMultMatrixf(&elem->matrix.matrix[0][0]);

    int stride;
    if (elem->skeletized) {
        stride = elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(modelInstance.bonesC, modelInstance.bonesM, modelInstance.bonesQ, elem, false);
        g_AnimSkeletal.SetElement(elem, &instance);
    }
    else {
        stride = elem->textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer (3, GL_FLOAT, stride, elem->renderData.verticesP);
    }

    if (selectionMode == Renderer::smVertex)
        for (GLuint i=0; i < elem->verticesC; ++i)
        {
            glLoadName(i);
            glDrawArrays(GL_POINTS, i, 1);
        }
    else
    if (selectionMode == Renderer::smElement || selectionMode == Renderer::smModel)
        glDrawElements (GL_TRIANGLES, 3*elem->facesC, GL_UNSIGNED_SHORT, elem->renderData.facesP);
    else { // if (selectionMode == smNone) {
        xColor cNotSkinned; cNotSkinned.init(0.8f, 0.8f, 0.f, 1.f);
        xColor cSkinned;    cSkinned.init(0.f, 0.8f, 0.8f, 1.f);
        xColor cSelected;   cSelected.init(1.f, 0.f, 0.f, 1.f);

        xColor *colors = new xColor[elem->renderData.verticesC];
        xColor *citer  = colors;
        xBYTE  *ptr = (xBYTE*) elem->renderData.verticesP;

        for (GLuint i=0; i < elem->renderData.verticesC; ++i, ++citer, ptr += stride)
        {
            if ( !elem->skeletized || ((xVertexSkel*)ptr)->b0 == 0.1f )
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
        glDrawArrays        (GL_POINTS, 0, elem->renderData.verticesC);
        glDisableClientState(GL_COLOR_ARRAY);
        delete[] colors;
    }

    if (elem->skeletized)
        g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

void xRenderGL :: RenderVertices( xModel &model, xModelInstance &instance,
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
    glMultMatrixf(&instance.location.x0);

    glColor4f( 0.8f, 0.8f, 0.f, 1.f );
    GLShader::EnableTexturing(xState_Disable);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    if (selectionMode != smNone) g_AnimSkeletal.ForceSoftware(true);

    for (xElement *elem = model.kidsP; elem; elem = elem->nextP)
        if (UseVBO)
            RenderElementVerticesVBO(selectionMode, selectedElement, selectedVertices, elem, instance);
        else
            RenderElementVerticesLST(selectionMode, selectedElement, selectedVertices, elem, instance);

    if (selectionMode != smNone) g_AnimSkeletal.ForceSoftware(false);

    glDisable(GL_COLOR_MATERIAL);
    GLShader::EnableTexturing(xState_Enable);
    glPopMatrix();
    
    glPolygonMode(GL_FRONT, prevMode[0]);
    glPolygonMode(GL_BACK,  prevMode[1]);
    glPointSize(prevSize);
    glDisableClientState(GL_VERTEX_ARRAY);
}


/********************************* model ************************************/
void RenderModelLST(bool transparent, const xFieldOfView &FOV,
                    xElement *elem, xModelInstance &modelInstance, bool UseList)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderModelLST(transparent, FOV, selem, modelInstance, UseList);

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
    if (UseList && listID)
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

void RenderModelVBO(bool transparent, const xFieldOfView &FOV,
                    xElement *elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderModelVBO(transparent, FOV, selem, modelInstance);

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
        /************************* LOAD NORMALS ****************************/
        if (GLShader::NeedNormals() && elem->renderData.normalP) {
            glBindBufferARB ( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.normalB );
            glNormalPointer ( GL_FLOAT, sizeof(xVector3), 0 );
            glEnableClientState(GL_NORMAL_ARRAY);
            glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.gpuMain.vertexB );
        }
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
    if (elem->renderData.normalP && GLShader::NeedNormals())
        glDisableClientState(GL_NORMAL_ARRAY);
    if (elem->skeletized)
        g_AnimSkeletal.EndAnimation();
    GLShader::EnableSkeleton(xState_Off);

    glPopMatrix();
}

void xRenderGL :: RenderModel(xModel &model, xModelInstance &instance,
                              bool transparent, const xFieldOfView &FOV)
{
    if ((transparent  && !model.transparent) ||
        (!transparent && !model.opaque)) return;
    
    InitTextures(model);
    glEnableClientState(GL_VERTEX_ARRAY);

    glPushMatrix();
    glMultMatrixf(&instance.location.x0);
            
    for (xElement *elem = model.kidsP; elem; elem = elem->nextP)
        // NOTE: MIX of display lists and VBO appears to be much slower than VBO only
        if (UseVBO)
            RenderModelVBO(transparent, FOV, elem, instance);
        else
            RenderModelLST(transparent, FOV, elem, instance, UseList);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
}

