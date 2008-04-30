#include "xRenderGL.h"

#include "../../OpenGL/GLAnimSkeletal.h"
#include "../../OpenGL/Textures/TextureMgr.h"

void xRenderGL :: Finalize()
{
    if (xModel)
    {
        if (xModel->texturesInited)
            for (xMaterial *mat = xModel->materialP; mat; mat = mat->nextP)
                if (mat->texture.name && mat->texture.htex)
                {
                    HTexture htex;
                    htex.SetHandle(mat->texture.htex);
                    g_TextureMgr.DeleteTexture(htex);
                    mat->texture.htex = 0;
                }
    }
    xRender::Finalize();
}
void xRenderGL :: InvalidateElementRenderData(xElement *elem)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        InvalidateElementRenderData(selem);
    elem->renderData.vertexB = 0;
    elem->renderData.normalB = 0;
    elem->renderData.indexB = 0;
    elem->renderData.mode = xRENDERMODE_NULL;
}

void xRenderGL :: FreeElementRenderData(xElement *elem, bool listOnly)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        FreeElementRenderData(selem, listOnly);

    if (listOnly && elem->renderData.mode != xRENDERMODE_LIST) return;

    if (elem->renderData.mode == xRENDERMODE_VBO)
    {
        GLuint p = elem->renderData.vertexB;
        if (p) glDeleteBuffersARB(1, &p);
        p = elem->renderData.normalB;
        if (p) glDeleteBuffersARB(1, &p);
        p = elem->renderData.indexB;
    }
    else
    if (elem->renderData.mode == xRENDERMODE_LIST && elem->renderData.listID)
        glDeleteLists(elem->renderData.listID, 1);
    elem->renderData.vertexB = 0;
    elem->renderData.normalB = 0;
    elem->renderData.indexB = 0;
    elem->renderData.mode = xRENDERMODE_NULL;
}

static xVector3 OrthoPoint(const xVector3 &source)
{
    if (IsZero(source.x))
        return xVector3::Create(1.f,0.f,0.f);
    if (IsZero(source.y))
        return xVector3::Create(0.f,1.f,0.f);
    if (IsZero(source.z))
        return xVector3::Create(0.f,0.f,1.f);

    return xVector3::Create(source.y*source.z, source.x*source.z, -2.f*source.x*source.y);
    //xVector3 dest(source.x, source.y, 0.f);
    //dest.z = -(source.x*source.x+source.y*source.y)/source.z;
    //return dest;
}

void xRenderGL :: RenderSkeleton(bool selectionRendering, xWORD selBoneId)
{
    if (xModel->spineP)
    {
        if (!bonesC && xModel->spineP)
            CalculateSkeleton();

        if (selectionRendering) g_AnimSkeletal.ForceSoftware(true);

        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(bonesC, bonesM, bonesQ);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        g_TextureMgr.DisableTextures();
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);

        glColor4f(1.0f,1.0f,0.0f,1.0f);
        glBegin(GL_TRIANGLES);
            RenderBone(xModel->spineP, selectionRendering, selBoneId);
        glEnd();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    glDisable(GL_COLOR_MATERIAL);

        g_AnimSkeletal.EndAnimation();

        if (selectionRendering) g_AnimSkeletal.ForceSoftware(false);
    }
}

void xRenderGL :: RenderBone(const xBone * bone, bool selectionRendering, xWORD selBoneId)
{
    static xFLOAT4 boneWght = { 0.1f, 0.0f, 0.0f, 0.0f };

    for (xBone *boneP = bone->kidsP; boneP != NULL; boneP = boneP->nextP)
    {
        if (selectionRendering || boneP->id == selBoneId)
        {
            glEnd();
            if (selectionRendering)
                glLoadName(boneP->id);
            else
            if (boneP->id == selBoneId)
                glColor4f(1.0f,0.0f,1.0f,1.0f);
            glBegin(GL_TRIANGLES);
        }

        xVector3 halfVector = (boneP->ending - bone->ending) / 5.0f;
        xVector3 pointHalf  = bone->ending + halfVector;
        xVector3 point3 = pointHalf + OrthoPoint(halfVector).normalize() / 10.0f;

        halfVector.normalize();
        float s = sin(PI/4.0f);
        xVector4 q; q.Init(halfVector.x*s, halfVector.y*s, halfVector.z*s, cos(PI/4.0f));

    for (int i=0; i<4; ++i) {
            boneWght[0] = bone->id + 0.1f;
            g_AnimSkeletal.SetBoneIdxWghts(boneWght);
            g_AnimSkeletal.SetVertex(bone->ending.xyz);

            boneWght[0] = boneP->id + 0.1f;
            g_AnimSkeletal.SetBoneIdxWghts(boneWght);
            g_AnimSkeletal.SetVertex(boneP->ending.xyz);

            g_AnimSkeletal.SetVertex(point3.xyz);
            point3 = xQuaternion::rotate(q, point3-pointHalf)+pointHalf;
        }


        if (boneP->id == selBoneId) {
            glEnd();
            glColor4f(1.0f,1.0f,0.0f,1.0f);
            glBegin(GL_TRIANGLES);
        }

        RenderBone(boneP, selectionRendering, selBoneId);
    }
}

/********************************* common private ************************************/
void xRenderGL :: InitVBO(xElement *elem)
{
    if (elem->renderData.mode == xRENDERMODE_NULL)
    {
        elem->renderData.mode = xRENDERMODE_VBO;
        int stride = (elem->skeletized && elem->textured) ? sizeof(xVertexTexSkel)
            : (elem->skeletized) ? sizeof(xVertexSkel)
            : (elem->textured) ? sizeof(xVertexTex) : sizeof(xVertex);
        GLuint p;
        glGenBuffersARB(1, &p); elem->renderData.vertexB = p;
        glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.vertexB );
        glBufferDataARB( GL_ARRAY_BUFFER_ARB, stride*elem->renderData.verticesC, elem->renderData.verticesP, GL_STATIC_DRAW_ARB);

        glGenBuffersARB(1, &p); elem->renderData.indexB = p;
        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.indexB );
        glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(xWORD)*3*elem->facesC, elem->renderData.facesP, GL_STATIC_DRAW_ARB);
        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );

        if (elem->renderData.normalP) // if any face is smooth
        {
            glGenBuffersARB(1, &p); elem->renderData.normalB = p;
            glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.normalB );
            glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(xVector3)*elem->renderData.verticesC, elem->renderData.normalP, GL_STATIC_DRAW_ARB);
        }
        glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
    }
}

void xRenderGL :: SetMaterial(xColor color, xMaterial *mat)
{
    if (mat)
    {
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat->ambient.col);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat->diffuse.col);
        xVector4 spec = *(xVector4*)(mat->specular.col) * mat->shininess_level;
        glMaterialfv(GL_FRONT, GL_SPECULAR, spec.xyzw);

        float shininess = pow(2, mat->shininess_gloss);
        if (shininess > 128.0)
            shininess = 128.0;
        glMaterialf(GL_FRONT, GL_SHININESS, shininess);

        if (mat->texture.htex) // if texture is loaded
        {
            HTexture htext;
            htext.SetHandle(mat->texture.htex);
            g_TextureMgr.BindTexture(htext);
            g_TextureMgr.EnableTextures();
        }
        else
        {
            g_TextureMgr.DisableTextures();
            //float specular[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            //glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        }
        //glColor4f(mat->diffuse.r,mat->diffuse.g,mat->diffuse.b, 1.0f-mat->transparency);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
        glColor4fv(color.col);
        float ambient[4]  = {0.2f, 0.2f, 0.2f, 1.0f};
        float diffuse[4]  = {0.8f, 0.8f, 0.8f, 1.0f};
        float specular[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT, GL_AMBIENT,  ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 2.0f);
        g_TextureMgr.DisableTextures();
    }
}

/********************************* faces **************************************/
void xRenderGL :: RenderFaces( xWORD                 selectedElement,
                               std::vector<xDWORD> * facesToRender )
{
    glEnableClientState(GL_VERTEX_ARRAY);

    if (!bonesC && xModel->spineP)
        CalculateSkeleton();

    glColor4f( 0.8f, 0.8f, 0.f, 1.f );
    g_TextureMgr.DisableTextures();
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    for (xElement *elem = xModel->firstP; elem; elem = elem->nextP)
        if (UseVBO)
            RenderElementFacesVBO(elem, selectedElement, facesToRender);
        else
            RenderElementFacesLST(elem, selectedElement, facesToRender);

    glDisable(GL_COLOR_MATERIAL);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void xRenderGL :: RenderElementFacesVBO(
                            xElement            * elem,
                            xWORD                 selElementId,
                            std::vector<xDWORD> * facesToRender)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderElementFacesVBO(selem, selElementId, facesToRender);

    if (!elem->renderData.verticesC || elem->id != selElementId)
        return;

    /************************* INIT VBO ****************************/
    if (elem->renderData.mode == xRENDERMODE_NULL)
        InitVBO(elem);

    /************************* LOAD VERTICES ****************************/
    glPushMatrix();
    glMultMatrixf(&elem->matrix.matrix[0][0]);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.vertexB );
    int stride;
    if (elem->skeletized) {
        stride = elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(bonesC, bonesM, bonesQ);
        g_AnimSkeletal.SetElement(elem, true);
    }
    else {
        stride = elem->textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer (3, GL_FLOAT, stride, 0);
    }
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );

    /************************* RENDER ****************************/
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.indexB );

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
            xWORD3 **iterF = elem->collisionData.hierarchyP[i].facesP;
            for (int j = elem->collisionData.hierarchyP[i].facesC; j; --j, ++iterF)
                glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (GLvoid*) ((xDWORD)(*iterF) - (xDWORD)elem->facesP) );
        }
    */
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );

    if (elem->skeletized)
        g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

void xRenderGL :: RenderElementFacesLST(
                            xElement            * elem,
                            xWORD                 selElementId,
                            std::vector<xDWORD> * facesToRender)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderElementFacesLST(selem, selElementId, facesToRender);

    if (!elem->renderData.verticesC || elem->id != selElementId)
        return;

    glPushMatrix();
    glMultMatrixf(&elem->matrix.matrix[0][0]);

    int stride;
    if (elem->skeletized) {
        stride = elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(bonesC, bonesM, bonesQ);
        g_AnimSkeletal.SetElement(elem);
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
            xWORD3 **iterF = elem->collisionData.hierarchyP[i].facesP;
            for (int j = elem->collisionData.hierarchyP[i].facesC; j; --j, ++iterF)
                glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, *iterF);
        }
    */

    if (elem->skeletized)
        g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}

/********************************* vertices && element selections ************************************/
void xRenderGL :: RenderVertices( SelectionMode         selectionMode,
                                  xWORD                 selectedElement,
                                  std::vector<xDWORD> * selectedVertices )
{
    glEnableClientState(GL_VERTEX_ARRAY);
    GLint prevMode[2];
    GLfloat prevSize;
    glGetIntegerv(GL_POLYGON_MODE, prevMode);
    glGetFloatv(GL_POINT_SIZE, &prevSize);
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    glPointSize(5);

    if (selectionMode != smNone) g_AnimSkeletal.ForceSoftware(true);

    if (!bonesC && xModel->spineP)
        CalculateSkeleton();

    glColor4f( 0.8f, 0.8f, 0.f, 1.f );
    g_TextureMgr.DisableTextures();
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    for (xElement *elem = xModel->firstP; elem; elem = elem->nextP)
        if (UseVBO)
            RenderElementVerticesVBO(elem, selectionMode, selectedElement, selectedVertices);
        else
            RenderElementVerticesLST(elem, selectionMode, selectedElement, selectedVertices);

    if (selectionMode != smNone) g_AnimSkeletal.ForceSoftware(false);

    glDisable(GL_COLOR_MATERIAL);
    glPolygonMode(GL_FRONT, prevMode[0]);
    glPolygonMode(GL_BACK,  prevMode[1]);
    glPointSize(prevSize);
    glDisableClientState(GL_VERTEX_ARRAY);
}


void xRenderGL :: RenderElementVerticesVBO(
                            xElement            * elem,
                            SelectionMode         selectionMode,
                            xWORD                 selElementId,
                            std::vector<xDWORD> * selectedVertices)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderElementVerticesVBO(selem, selectionMode, selElementId, selectedVertices);

    if (!elem->renderData.verticesC || (selectionMode != smElement && elem->id != selElementId))
        return;

    /************************* INIT VBO ****************************/
    if (elem->renderData.mode == xRENDERMODE_NULL)
        InitVBO(elem);

    if (selectionMode == smElement)
        glLoadName(elem->id);
    /************************* LOAD VERTICES ****************************/
    glPushMatrix();
    glMultMatrixf(&elem->matrix.matrix[0][0]);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.vertexB );
    int stride;
    if (elem->skeletized) {
        stride = elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(bonesC, bonesM, bonesQ);
        g_AnimSkeletal.SetElement(elem, true);
    }
    else {
        stride = elem->textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer (3, GL_FLOAT, stride, 0);
    }
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );

    /************************* RENDER ****************************/
    if (selectionMode == smVertex)
        for (GLuint i=0; i < elem->renderData.verticesC; ++i)
        {
            glLoadName(i);
            glDrawArrays(GL_POINTS, i, 1);
        }
    else
    if (selectionMode == smElement) {
        glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.indexB );
        glDrawElements (GL_TRIANGLES, 3*elem->facesC, GL_UNSIGNED_SHORT, 0);
        glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    }
    else { //if (selectionMode == smNone) {
        xColor cNotSkinned(0.8f, 0.8f, 0.f, 1.f);
        xColor cSkinned   (0.f, 0.8f, 0.8f, 1.f);
        xColor cSelected  (1.f, 0.f, 0.f, 1.f);

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

void xRenderGL :: RenderElementVerticesLST(
                            xElement            * elem,
                            SelectionMode         selectionMode,
                            xWORD                 selElementId,
                            std::vector<xDWORD> * selectedVertices)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderElementVerticesLST(selem, selectionMode, selElementId, selectedVertices);

    if (!elem->renderData.verticesC || (selectionMode != smElement && elem->id != selElementId))
        return;

    if (selectionMode == smElement)
        glLoadName(elem->id);
    glPushMatrix();
    glMultMatrixf(&elem->matrix.matrix[0][0]);

    int stride;
    if (elem->skeletized) {
        stride = elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones(bonesC, bonesM, bonesQ);
        g_AnimSkeletal.SetElement(elem);
    }
    else {
        stride = elem->textured ? sizeof(xVertexTex) : sizeof(xVertex);
        glVertexPointer (3, GL_FLOAT, stride, elem->renderData.verticesP);
    }

    if (selectionMode == smVertex)
        for (GLuint i=0; i < elem->verticesC; ++i)
        {
            glLoadName(i);
            glDrawArrays(GL_POINTS, i, 1);
        }
    else
    if (selectionMode == smElement)
        glDrawElements (GL_TRIANGLES, 3*elem->facesC, GL_UNSIGNED_SHORT, elem->renderData.facesP);
    else { // if (selectionMode == smNone) {
        xColor cNotSkinned(0.8f, 0.8f, 0.f, 1.f);
        xColor cSkinned   (0.f, 0.8f, 0.8f, 1.f);
        xColor cSelected  (1.f, 0.f, 0.f, 1.f);

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

/********************************* model ************************************/
void xRenderGL :: RenderModel()
{
    assert(xModel);
    if (!xModel->texturesInited)
        for (xMaterial *mat = xModel->materialP; mat; mat = mat->nextP)
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
    xModel->texturesInited = true;

    if (!bonesC && xModel->spineP)
        CalculateSkeleton();

    m_currentMaterial = (xMaterial*)1;
    glEnableClientState(GL_VERTEX_ARRAY);
    // NOTE: SelectionRendering doesn't like custom shaders (speed!!!)
    if (g_SelectionRendering) g_AnimSkeletal.ForceSoftware(true);

    for (xElement *elem = xModel->firstP; elem; elem = elem->nextP)
        // NOTE: MIX of display lists and VBO appears to be much slower than VBO only
        if (UseVBO)
            RenderModelVBO(elem);
        else
            RenderModelLST(elem);

    if (g_SelectionRendering) g_AnimSkeletal.ForceSoftware(false);

    glDisableClientState(GL_VERTEX_ARRAY);
}

void xRenderGL :: RenderModelLST(xElement *elem)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderModelLST(selem);

    if (!elem->renderData.verticesC) return;

    if (g_SelectionRendering || !UseList || !elem->renderData.listID)
    {
        if (g_SelectionRendering) {
            glPushMatrix();
            glMultMatrixf(&elem->matrix.matrix[0][0]);
        }
        else
    if (UseList) {
        elem->renderData.listID = glGenLists(1);
            elem->renderData.mode = xRENDERMODE_LIST;
            glNewList(elem->renderData.listID, GL_COMPILE);
        }
        {
            if (elem->skeletized) {
                if (elem->textured && !g_SelectionRendering) {
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glTexCoordPointer (2, GL_FLOAT, sizeof(xVertexTexSkel), &(elem->renderData.verticesTSP->tx));
                }
                g_AnimSkeletal.BeginAnimation();
                g_AnimSkeletal.SetBones (bonesC, bonesM, bonesQ);
                g_AnimSkeletal.SetElement(elem);
            }
            else
            if (elem->textured) {
                glVertexPointer   (3, GL_FLOAT, sizeof(xVertexTex), elem->renderData.verticesTP);

                if (!g_SelectionRendering) {
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glTexCoordPointer (2, GL_FLOAT, sizeof(xVertexTex), &(elem->renderData.verticesTP->tx));
                }
            }
            else
                glVertexPointer   (3, GL_FLOAT, sizeof(xVertex), elem->renderData.verticesP);

            if (!g_SelectionRendering && elem->renderData.normalP)
                glNormalPointer (GL_FLOAT, sizeof(xVector3), elem->renderData.normalP);
            bool normArrayEnabled = false;

            xFaceList *faceL = elem->faceListP;
            for(int i=0; i<elem->faceListC; ++i, ++faceL)
            {
                if (!g_SelectionRendering && faceL->materialP != m_currentMaterial)
                    SetMaterial(elem->color, m_currentMaterial = faceL->materialP);

                if (faceL->smooth)
                {
                    if (!g_SelectionRendering && !normArrayEnabled) glEnableClientState(GL_NORMAL_ARRAY);
                    normArrayEnabled = true;
                    glDrawElements      (GL_TRIANGLES, 3*faceL->indexCount, GL_UNSIGNED_SHORT, elem->renderData.facesP+faceL->indexOffset);
                }
                else
                {
                    if (!g_SelectionRendering && normArrayEnabled) glDisableClientState(GL_NORMAL_ARRAY);
                    normArrayEnabled = false;

                    xWORD3   *iter   = elem->renderData.facesP + faceL->indexOffset;
                    xVector3 *normal = faceL->normalP;
                    for (int j=faceL->indexCount; j; --j, ++iter, ++normal)
                    {
                        glNormal3fv (normal->xyz);
                        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, iter);
                    }
                }
            }
            if (!g_SelectionRendering && normArrayEnabled) glDisableClientState(GL_NORMAL_ARRAY);

            if (!g_SelectionRendering && elem->textured)
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            if (elem->skeletized)
                g_AnimSkeletal.EndAnimation();
        }
        if (g_SelectionRendering)
            glPopMatrix();
        else
    if (UseList)
            glEndList();
    }

    if (!g_SelectionRendering && elem->renderData.listID)
    {
        glPushMatrix();
        {
            glMultMatrixf(&elem->matrix.matrix[0][0]);
            glCallList(elem->renderData.listID);
        }
        glPopMatrix();
    }
}

void xRenderGL :: RenderModelVBO(xElement *elem)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        RenderModelVBO(selem);

    if (!elem->renderData.verticesC) return;

    /************************* INIT VBO ****************************/
    if (elem->renderData.mode == xRENDERMODE_NULL)
        InitVBO(elem);

    /************************* LOAD VERTICES ****************************/
    glPushMatrix();
    glMultMatrixf(&elem->matrix.matrix[0][0]);

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, elem->renderData.vertexB );
    if (elem->skeletized) {
        if (elem->textured && !g_SelectionRendering) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer (2, GL_FLOAT, sizeof(xVertexTexSkel), (void *)(7*sizeof(xFLOAT)));
        }
        g_AnimSkeletal.BeginAnimation();
        g_AnimSkeletal.SetBones (bonesC, bonesM, bonesQ);
        g_AnimSkeletal.SetElement(elem, true);
    }
    else
    if (elem->textured) {
        glVertexPointer   (3, GL_FLOAT, sizeof(xVertexTex), 0);

        if (!g_SelectionRendering) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer (2, GL_FLOAT, sizeof(xVertexTex), (void *)(3*sizeof(xFLOAT)));
        }
    }
    else
        glVertexPointer   (3, GL_FLOAT, sizeof(xVertex), 0);

    /************************* LOAD NORMALS ****************************/
    if (!g_SelectionRendering && elem->renderData.normalP) {
        glBindBufferARB ( GL_ARRAY_BUFFER_ARB, elem->renderData.normalB );
        glNormalPointer ( GL_FLOAT, sizeof(xVector3), 0 );
    }
    bool lastSmooth = false;

    /************************* RENDER FACES ****************************/
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, elem->renderData.indexB );
    xFaceList *faceL = elem->faceListP;
    for(int i=elem->faceListC; i; --i, ++faceL)
    {
        if (!g_SelectionRendering && faceL->materialP != m_currentMaterial)
            SetMaterial(elem->color, m_currentMaterial = faceL->materialP);

        if (faceL->smooth)
        {
            if (!g_SelectionRendering && !lastSmooth) glEnableClientState(GL_NORMAL_ARRAY);
            lastSmooth = true;
            glDrawElements(GL_TRIANGLES, 3*faceL->indexCount, GL_UNSIGNED_SHORT, (void*)(faceL->indexOffset*3*sizeof(xWORD)));
        }
        else
        {
            if (!g_SelectionRendering && lastSmooth) glDisableClientState(GL_NORMAL_ARRAY);
            lastSmooth = false;

            xDWORD offset = 3*sizeof(xWORD)*faceL->indexOffset;
            xDWORD end    = offset + 3*sizeof(xWORD)*faceL->indexCount;
            xVector3 *normal   = faceL->normalP;
            for (; offset != end; offset += 3*sizeof(xWORD), ++normal)
            {
                glNormal3fv (normal->xyz);
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (void*)offset);
            }
        }
    }
    glBindBufferARB ( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB ( GL_ARRAY_BUFFER_ARB, 0 );

    if (!g_SelectionRendering && lastSmooth)
        glDisableClientState(GL_NORMAL_ARRAY);
    if (!g_SelectionRendering && elem->textured)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    if (elem->skeletized)
        g_AnimSkeletal.EndAnimation();

    glPopMatrix();
}
