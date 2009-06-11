#include "xElementRunTime.h"

xShadowData & xElementInstance :: GetShadowData(xLight &light, xShadowData::xShadowDataLevel zLevel)
{
    xShadowDataVector::iterator iter = gpuShadows.begin(), iterE = gpuShadows.end();
    for (; iter != iterE; ++iter)
        if (iter->ID_light == light.id)
        {
            if (light.modified)
            {
                iter->FreeData();
                iter->zDataLevel = zLevel;
            }
            else
            if (iter->zDataLevel < zLevel)
            {
                iter->InvalidateData();
                iter->zDataLevel = zLevel;
            }
            return *iter;
        }

    xShadowData newData;
    memset(&newData, 0, sizeof(xShadowData));
    newData.ID_light = light.id;
    newData.zDataLevel = zLevel;
    gpuShadows.push_back(newData);
    return gpuShadows.back();
}

void xElementInstance :: Zero()
{
    mode = xGPUPointers::NONE;

    memset (&gpuMain, 0, sizeof(xGPUPointers));
    gpuShadows.clear();

    bBox.P_center.zero();
    bBox.S_top = bBox.S_side = bBox.S_front = 0.f;
    bSphere.P_center.zero();
    bSphere.S_radius = 0.f;

    bBox_T    = NULL;
    bSphere_T = NULL;

    I_vertices = 0;
    L_vertices = NULL;
    L_normals  = NULL;
}

void xElementInstance :: Free()
{
    xShadowDataVector::iterator iter = gpuShadows.begin(), iterE = gpuShadows.end();
    for (; iter != iterE; ++iter)
        iter->FreeData();

    if (L_vertices) { delete[] L_vertices; L_vertices = NULL; }
    if (L_normals)  { delete[] L_normals;  L_normals  = NULL; }

    if (bBox_T)    { delete bBox_T;    bBox_T = NULL; }
    if (bSphere_T) { delete bSphere_T; bSphere_T = NULL; }
}

void xElementInstance :: Clear()
{
    Free();
    gpuShadows.clear();
}

void xElementInstance :: InvalidateVertexData()
{
    xShadowDataVector::iterator iter = gpuShadows.begin(), iterE = gpuShadows.end();
    for (; iter != iterE; ++iter)
        iter->InvalidateData();

    if (bBox_T)    { delete bBox_T;    bBox_T = NULL; }
    if (bSphere_T) { delete bSphere_T; bSphere_T = NULL; }
}

////////////////////// xRenderData

#include "xElement.h"

void xRenderData :: Destroy(const xElement &elem)
{
    if (this->L_faces && elem.L_faces != this->L_faces)
        delete[] this->L_faces;
    if (this->L_vertices && elem.L_vertices != this->L_vertices)
        delete[] this->L_vertices;
    if (this->L_colors && elem.L_colors != this->L_colors)
        delete[] this->L_colors;
    if (this->L_normals)
        delete[] this->L_normals;
    if (this->L_face_normals)
        delete[] this->L_face_normals;
    Clear();
}

void xRenderData :: Load (FILE *file, xElement &elem)
{
    Destroy(elem);

    fread(&this->I_vertices, sizeof(xWORD), 1, file);
    // vertices
    bool FL_vertices;
    fread(&FL_vertices, sizeof(bool), 1, file);
    if (FL_vertices)
    {
        size_t stride = elem.GetVertexStride();
        this->L_vertices = (xVertex*) new xBYTE[stride*this->I_vertices];
        fread(this->L_vertices, stride, this->I_vertices, file);
    }
    else
    {
        this->I_vertices = elem.I_vertices;
        this->L_vertices = elem.L_vertices;
    }
    // normals
    bool FL_normals;
    fread(&FL_normals, sizeof(bool), 1, file);
    if (FL_normals)
    {
        this->L_normals = new xVector3[this->I_vertices];
        fread(this->L_normals, sizeof(xVector3), this->I_vertices, file);
    }
    // faces
    bool FL_faces;
    fread(&FL_faces, sizeof(bool), 1, file);
    if (FL_faces)
    {
        this->L_faces = new xFace[elem.I_faces];
        fread(this->L_faces, sizeof(xFace), elem.I_faces, file);
    }
    else
        this->L_faces = elem.L_faces;
    // face normals
    bool FL_faceNormals;
    fread(&FL_faceNormals, sizeof(bool), 1, file);
    if (FL_faceNormals)
    {
        this->L_face_normals = new xVector3[elem.I_faces];
        fread(this->L_face_normals, sizeof(xVector3), elem.I_faces, file);
    }
}

void xRenderData :: Save (FILE *file, const xElement &elem) const
{
    fwrite(&this->I_vertices, sizeof(xWORD), 1, file);
    // vertices
    bool FL_vertices = this->I_vertices && this->L_vertices && this->L_vertices != elem.L_vertices;
    fwrite(&FL_vertices, sizeof(bool), 1, file);
    if (FL_vertices)
    {
        size_t stride = elem.GetVertexStride();
        fwrite(this->L_vertices, stride, this->I_vertices, file);
    }
    // normals
    bool FL_normals = this->I_vertices && this->L_normals;
    fwrite(&FL_normals, sizeof(bool), 1, file);
    if (FL_normals)
        fwrite(this->L_normals, sizeof(xVector3), this->I_vertices, file);

    // faces
    bool FL_faces = elem.I_faces && this->L_faces && this->L_faces != elem.L_faces;
    fwrite(&FL_faces, sizeof(bool), 1, file);
    if (FL_faces)
        fwrite(this->L_faces, sizeof(xFace), elem.I_faces, file);
    // face normals
    bool FL_faceNormals = elem.I_faces && this->L_face_normals;
    fwrite(&FL_faceNormals, sizeof(bool), 1, file);
    if (FL_faceNormals)
        fwrite(this->L_face_normals, sizeof(xVector3), elem.I_faces, file);
}
