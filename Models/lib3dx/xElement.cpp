#include "xElement.h"

////////////////////// xElement
xElement * xElement :: ById  (xBYTE eid)
{
    xElement *relem;
    for (xElement *elem = this; elem; elem = elem->Next)
    {
        if (elem->ID == eid)
            return elem;
        if ( elem->L_kids && (relem = elem->L_kids->ById(eid)) )
            return relem;
    }
    return NULL;
}

xBYTE      xElement :: CountAll ()
{
    xWORD res = 0;
    for (xElement *elem = this; elem; elem = elem->Next)
        res += 1 + ( elem->L_kids ? elem->L_kids->CountAll() : 0);
    return res;
}

void      xElement :: Free()
{
    if (this->renderData.L_faces && this->renderData.L_faces != this->L_faces)
        delete[] this->renderData.L_faces;
    if (this->renderData.L_vertices && this->renderData.L_vertices != this->L_vertices)
        delete[] this->renderData.L_vertices;
    if (this->renderData.L_normals)
        delete[] this->renderData.L_normals;
    if (this->renderData.L_face_normals)
        delete[] this->renderData.L_face_normals;
    if (this->Name)
        delete[] this->Name;
    if (this->L_vertices) {
        if (this->FL_textured && this->FL_skeletized)
            delete[] this->L_verticesTS;
        else
        if (this->FL_textured)
            delete[] this->L_verticesT;
        else
        if (this->FL_skeletized)
            delete[] this->L_verticesS;
        else
            delete[] this->L_vertices;
    }
    if (this->L_faces)
        delete[] this->L_faces;
    if (this->L_smooth)
        delete[] this->L_smooth;
    if (this->L_faceLists)
        delete[] this->L_faceLists;
    if (this->L_edges)
        delete[] this->L_edges;
    xElement *curr = this->L_kids, *next;
    while (curr)
    {
        next = curr->Next;
        curr->Free();
        curr = next;
    }

    this->collisionData.FreeKids();

    delete this;
}

#include "xModel.h"

void xElement :: FillCollisionInfo (xModel &xmodel)
{
    for (xElement *elem = this; elem; elem = elem->Next)
    {
        elem->collisionData.Fill(xmodel, *elem);
        if ( elem->L_kids )
            elem->L_kids->FillCollisionInfo(xmodel);
    }
}

xElement *xElement :: Load (FILE *file, xModel *xmodel, bool FL_create_CollisionInfo)
{
    xElement *elem = new xElement();
    if (!elem) return NULL;

    fread(&elem->ID,            sizeof(xBYTE), 1, file);
    fread(&elem->Name,          sizeof(char*), 1, file);
    fread(&elem->Color,         sizeof(xColor), 1, file);
    fread(&elem->MX_MeshToLocal,sizeof(xMatrix), 1, file);
    fread(&elem->I_vertices,    sizeof(xWORD), 1, file);
    fread(&elem->I_faces,       sizeof(xWORD), 1, file);
    fread(&elem->I_faceLists,   sizeof(xWORD), 1, file);
    fread(&elem->FL_textured,   sizeof(bool), 1, file);
    fread(&elem->FL_skeletized, sizeof(bool), 1, file);
    fread(&elem->I_kids,        sizeof(xBYTE), 1, file);

    //xWORD2     *L_edges;
    //xRenderData renderData;

    memset(&(elem->renderData),    0, sizeof(elem->renderData));
    memset(&(elem->collisionData), 0, sizeof(elem->collisionData));

    elem->L_vertices = NULL;
    elem->L_smooth   = NULL;
    elem->L_faces    = NULL;
    elem->L_faceLists = NULL;
    elem->L_edges    = NULL;
    elem->I_edges   = 0;
    elem->L_kids = NULL;
    elem->Next = NULL;
    elem->FL_transparent = elem->FL_opaque = false;

    if (elem->Name)
    {
        size_t len = (size_t) elem->Name;
        elem->Name = new char[len];
        if (!(elem->Name) ||
            fread(elem->Name, 1, len, file) != len)
        {
            elem->Free();
            return NULL;
        }
    }
    if (elem->I_vertices)
    {
        size_t stride = elem->FL_skeletized
            ? elem->FL_textured
            ? sizeof (xVertexTexSkel)
            : sizeof (xVertexSkel)
            : elem->FL_textured
            ? sizeof (xVertexTex)
            : sizeof (xVertex);
        elem->L_vertices = (xVertex*) new xBYTE[stride*elem->I_vertices];
        if (!elem->L_verticesTS ||
                fread(elem->L_vertices, stride, elem->I_vertices, file)
                    != elem->I_vertices)
        {
            elem->Free();
            return NULL;
        }
    }
    if (elem->I_faces)
    {
        elem->L_faces = (xWORD(*)[3]) new xWORD[3*elem->I_faces];
        if (!elem->L_faces ||
            fread(elem->L_faces, 3*sizeof(xWORD), elem->I_faces, file)
                != elem->I_faces)
        {
            elem->Free();
            return NULL;
        }
        elem->L_smooth = new xDWORD[elem->I_faces];
        if (!elem->L_smooth ||
            fread(elem->L_smooth, sizeof(xDWORD), elem->I_faces, file)
                != elem->I_faces)
        {
            elem->Free();
            return NULL;
        }
    }
    if (elem->I_faceLists)
    {
        elem->L_faceLists = new xFaceList[elem->I_faceLists];
        if (!elem->L_faceLists ||
            fread(elem->L_faceLists, sizeof(xFaceList), elem->I_faceLists, file)
                != elem->I_faceLists)
        {
            elem->Free();
            return NULL;
        }

        xFaceList *faceL = elem->L_faceLists;
        for (int fL=elem->I_faceLists; fL; --fL, ++faceL)
        {
            if (xmodel->L_material)
                faceL->Material = xmodel->L_material->ById(faceL->ID_material);
            else
                faceL->Material = NULL;
            if (faceL->Material)
            {
                bool transparent = faceL->Material->transparency > 0.f;
                elem->FL_transparent |= transparent;
                elem->FL_opaque      |= !transparent;
            }
            else
                xmodel->FL_opaque = elem->FL_opaque = true;
        }
        elem->CalculateSmoothVertices();
    }

    elem->FillShadowEdges();
    if (elem->I_kids)
    {
        xElement *last = NULL;
        for (int i=0; i < elem->I_kids; ++i)
        {
            if (last)
                last = last->Next = xElement::Load(file, xmodel, FL_create_CollisionInfo);
            else
                last = elem->L_kids = xElement::Load(file, xmodel, FL_create_CollisionInfo);
            if (!last)
            {
                elem->Free();
                return NULL;
            }
        }
    }

    if (xmodel->FL_save_cinfo)
        elem->collisionData.Load(file, elem);

    return elem;
}

void      xElement :: Save(FILE *file, const xModel *xmodel)
{
    char *name = this->Name;
    if (this->Name)
        this->Name = (char *) strlen(name)+1;

    fwrite(&this->ID,            sizeof(xBYTE), 1, file);
    fwrite(&this->Name,          sizeof(char*), 1, file);
    fwrite(&this->Color,         sizeof(xColor), 1, file);
    fwrite(&this->MX_MeshToLocal,sizeof(MX_MeshToLocal), 1, file);
    fwrite(&this->I_vertices,    sizeof(xWORD), 1, file);
    fwrite(&this->I_faces,       sizeof(xWORD), 1, file);
    fwrite(&this->I_faceLists,   sizeof(xWORD), 1, file);
    fwrite(&this->FL_textured,   sizeof(bool), 1, file);
    fwrite(&this->FL_skeletized, sizeof(bool), 1, file);
    fwrite(&this->I_kids,        sizeof(xBYTE), 1, file);

    //xWORD2     *L_edges;
    //xRenderData renderData;

    if (name)
        fwrite(name, 1, (size_t)this->Name, file);
    if (this->I_vertices) {
        size_t stride = this->FL_skeletized
                ? this->FL_textured
                ? sizeof (xVertexTexSkel)
                : sizeof (xVertexSkel)
                : this->FL_textured
                ? sizeof (xVertexTex)
                : sizeof (xVertex);
        fwrite(this->L_vertices, stride, this->I_vertices, file);
    }
    if (this->I_faces)
    {
        fwrite(this->L_faces, 3*sizeof(xWORD), this->I_faces, file);
        fwrite(this->L_smooth, sizeof(xDWORD), this->I_faces, file);
    }
    if (this->I_faceLists)
        fwrite(this->L_faceLists, sizeof(xFaceList), this->I_faceLists, file);
    if (this->I_kids)
    {
        xElement *last = this->L_kids;
        for (; last; last = last->Next)
            last->Save(file, xmodel);
    }

    this->Name = name;

    if (xmodel->FL_save_cinfo)
        this->collisionData.Save(file, this);
}

/////// SKIN VERTICES

xSkinnedData xElement :: GetSkinnedVertices(const xMatrix *bones) const
{
    xWORD     count  = renderData.I_vertices;
    xBYTE    *srcV   = (xBYTE *) renderData.L_vertices;
    xVector3 *srcN   = renderData.L_normals;

    xSkinnedData dst;
    dst.L_vertices = new xVector3[count];
    dst.L_normals  = new xVector3[count];
    xVector3 *itrV = dst.L_vertices;
    xVector3 *itrN = dst.L_normals;

    if (FL_skeletized)
    {
        xDWORD stride = FL_textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        for (int i = count; i > 0; --i, ++itrV, ++itrN, srcV += stride, ++srcN)
        {
            xVertexSkel *vert = (xVertexSkel *)srcV;

            xVector4 vec;  vec.init(* (xVector3 *)vert->pos, 1.f);
            xVector4 nor;  nor.init(* srcN, 0.f);
            xMatrix  bone;

            itrV->init(0.f, 0.f, 0.f);
            itrN->init(0.f, 0.f, 0.f);
            for (int b=0; b<4; ++b)
            {
                int   i = (int) floor(vert->bone[b]);
                float w = (vert->bone[b] - i)*10;
                bone = bones[i] * w;
                *itrV  += (bone * vec).vector3;
                *itrN  += (bone * nor).vector3;
            }
        }
    }
    else
    {
        xDWORD stride = FL_textured ? sizeof(xVertexTex) : sizeof(xVertex);
        for (int i = count; i > 0; --i, ++itrV, srcV += stride)
            *itrV = *(xVector3 *)srcV;
        memcpy(dst.L_normals, renderData.L_normals, sizeof(xVector3)*count);
    }
    return dst;
}
