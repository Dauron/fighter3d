#include "xElement.h"

////////////////////// xElement
xElement * xElement :: ById  (xBYTE eid)
{
    xElement *relem;
    for (xElement *elem = this; elem; elem = elem->nextP)
    {
        if (elem->id == eid)
            return elem;
        if ( elem->kidsP && (relem = elem->kidsP->ById(eid)) )
            return relem;
    }
    return NULL;
}

xBYTE      xElement :: CountAll ()
{
    xWORD res = 0;
    for (xElement *elem = this; elem; elem = elem->nextP)
        res += 1 + ( elem->kidsP ? elem->kidsP->CountAll() : 0);
    return res;
}

void      xElement :: Free()
{
    if (this->renderData.facesP && this->renderData.facesP != this->facesP)
        delete[] this->renderData.facesP;
    if (this->renderData.verticesP && this->renderData.verticesP != this->verticesP)
        delete[] this->renderData.verticesP;
    if (this->renderData.normalP)
        delete[] this->renderData.normalP;
    if (this->renderData.faceNormalsP)
        delete[] this->renderData.faceNormalsP;
    if (this->name)
        delete[] this->name;
    if (this->verticesP) {
        if (this->textured && this->skeletized)
            delete[] this->verticesTSP;
        else
        if (this->textured)
            delete[] this->verticesTP;
        else
        if (this->skeletized)
            delete[] this->verticesSP;
        else
            delete[] this->verticesP;
    }
    if (this->facesP)
        delete[] this->facesP;
    if (this->smoothP)
        delete[] this->smoothP;
    if (this->faceListP)
        delete[] this->faceListP;
    if (this->edgesP)
        delete[] this->edgesP;
    xElement *curr = this->kidsP, *next;
    while (curr)
    {
        next = curr->nextP;
        curr->Free();
        curr = next;
    }

    this->collisionData.FreeKids();

    delete this;
}

#include "xModel.h"

xElement *xElement :: Load (FILE *file, xModel *xmodel, bool createCollisionInfo)
{
    xElement *elem = new xElement();
    if (!elem) return NULL;

    fread(&elem->id,         sizeof(xBYTE), 1, file);
    fread(&elem->name,       sizeof(char*), 1, file);
    fread(&elem->color,      sizeof(xColor), 1, file);
    fread(&elem->matrix,     sizeof(xMatrix), 1, file);
    fread(&elem->verticesC,  sizeof(xWORD), 1, file);
    fread(&elem->facesC,     sizeof(xWORD), 1, file);
    fread(&elem->faceListC,  sizeof(xWORD), 1, file);
    fread(&elem->textured,   sizeof(bool), 1, file);
    fread(&elem->skeletized, sizeof(bool), 1, file);
    fread(&elem->kidsC,      sizeof(xBYTE), 1, file);

    //xWORD2     *edgesP;
    //xRenderData renderData;

    memset(&(elem->renderData),    0, sizeof(elem->renderData));
    memset(&(elem->collisionData), 0, sizeof(elem->collisionData));

    elem->verticesP = NULL;
    elem->smoothP   = NULL;
    elem->facesP    = NULL;
    elem->faceListP = NULL;
    elem->edgesP    = NULL;
    elem->edgesC    = 0;
    elem->kidsP = NULL;
    elem->nextP = NULL;

    if (elem->name)
    {
        size_t len = (size_t) elem->name;
        elem->name = new char[len];
        if (!(elem->name) ||
            fread(elem->name, 1, len, file) != len)
        {
            elem->Free();
            return NULL;
        }
    }
    if (elem->verticesC)
    {
        size_t stride = elem->skeletized
            ? elem->textured
            ? sizeof (xVertexTexSkel)
            : sizeof (xVertexSkel)
            : elem->textured
            ? sizeof (xVertexTex)
            : sizeof (xVertex);
        elem->verticesP = (xVertex*) new xBYTE[stride*elem->verticesC];
        if (!elem->verticesTSP ||
                fread(elem->verticesP, stride, elem->verticesC, file)
                    != elem->verticesC)
        {
            elem->Free();
            return NULL;
        }
    }
    if (elem->facesC)
    {
        elem->facesP = (xWORD(*)[3]) new xWORD[3*elem->facesC];
        if (!elem->facesP ||
            fread(elem->facesP, 3*sizeof(xWORD), elem->facesC, file)
                != elem->facesC)
        {
            elem->Free();
            return NULL;
        }
        elem->smoothP = new xDWORD[elem->facesC];
        if (!elem->smoothP ||
            fread(elem->smoothP, sizeof(xDWORD), elem->facesC, file)
                != elem->facesC)
        {
            elem->Free();
            return NULL;
        }
    }
    if (elem->faceListC)
    {
        elem->faceListP = new xFaceList[elem->faceListC];
        if (!elem->faceListP ||
            fread(elem->faceListP, sizeof(xFaceList), elem->faceListC, file)
                != elem->faceListC)
        {
            elem->Free();
            return NULL;
        }

        xFaceList *faceL = elem->faceListP;
        for (int fL=elem->faceListC; fL; --fL, ++faceL)
        {
            if (xmodel->materialP)
                faceL->materialP = xmodel->materialP->ById(faceL->materialId);
            else
                faceL->materialP = NULL;
            if (faceL->materialP)
            {
                bool transparent = faceL->materialP->transparency > 0.f;
                elem->transparent |= transparent;
                elem->opaque      |= !transparent;
            }
            else
                xmodel->opaque = elem->opaque = true;
        }
        elem->CalculateSmoothVertices();
    }

    elem->FillShadowEdges();
    if (elem->kidsC)
    {
        xElement *last = NULL;
        for (int i=0; i < elem->kidsC; ++i)
        {
            if (last)
                last = last->nextP = xElement::Load(file, xmodel, createCollisionInfo);
            else
                last = elem->kidsP = xElement::Load(file, xmodel, createCollisionInfo);
            if (!last)
            {
                elem->Free();
                return NULL;
            }
        }
    }

    if (xmodel->saveCollisionData)
        elem->collisionData.Load(file, elem);
    else
    if (createCollisionInfo)
        elem->collisionData.Fill(xmodel, elem);

    return elem;
}

void      xElement :: Save(FILE *file, const xModel *xmodel)
{
    char *name = this->name;
    if (this->name)
        this->name = (char *) strlen(name)+1;

    fwrite(&this->id,         sizeof(xBYTE), 1, file);
    fwrite(&this->name,       sizeof(char*), 1, file);
    fwrite(&this->color,      sizeof(xColor), 1, file);
    fwrite(&this->matrix,     sizeof(xMatrix), 1, file);
    fwrite(&this->verticesC,  sizeof(xWORD), 1, file);
    fwrite(&this->facesC,     sizeof(xWORD), 1, file);
    fwrite(&this->faceListC,  sizeof(xWORD), 1, file);
    fwrite(&this->textured,   sizeof(bool), 1, file);
    fwrite(&this->skeletized, sizeof(bool), 1, file);
    fwrite(&this->kidsC,      sizeof(xBYTE), 1, file);

    //xWORD2     *edgesP;
    //xRenderData renderData;

    if (name)
        fwrite(name, 1, (size_t)this->name, file);
    if (this->verticesC) {
        size_t stride = this->skeletized
                ? this->textured
                ? sizeof (xVertexTexSkel)
                : sizeof (xVertexSkel)
                : this->textured
                ? sizeof (xVertexTex)
                : sizeof (xVertex);
        fwrite(this->verticesP, stride, this->verticesC, file);
    }
    if (this->facesC)
    {
        fwrite(this->facesP, 3*sizeof(xWORD), this->facesC, file);
        fwrite(this->smoothP, sizeof(xDWORD), this->facesC, file);
    }
    if (this->faceListC)
        fwrite(this->faceListP, sizeof(xFaceList), this->faceListC, file);
    if (this->kidsC)
    {
        xElement *last = this->kidsP;
        for (; last; last = last->nextP)
            last->Save(file, xmodel);
    }

    this->name = name;

    if (xmodel->saveCollisionData)
        this->collisionData.Save(file, this);
}
