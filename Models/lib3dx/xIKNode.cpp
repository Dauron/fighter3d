#include "xIKNode.h"

void xIKNode :: JoinBAdd(xBYTE joinId)
{
    xBYTE *joins = new xBYTE[joinsBC+1];
    memcpy (joins, joinsBP, sizeof(xBYTE)*joinsBC);
    joins[joinsBC] = joinId;

    delete[] joinsBP;
    joinsBP = joins;
    ++joinsBC;
}
void xIKNode :: JoinEAdd(xBYTE joinId)
{
    xBYTE *joins = new xBYTE[joinsEC+1];
    memcpy (joins, joinsEP, sizeof(xBYTE)*joinsEC);
    joins[joinsEC] = joinId;

    delete[] joinsEP;
    joinsEP = joins;
    ++joinsEC;
}
void xIKNode :: JoinBDelete(xBYTE joinId)
{
    xBYTE *join = joinsBP;
    for (int i = 0; i < joinsBC; ++i, ++join)
        if (*join == joinId)
        {
            --joinsBC;
            xBYTE *joins = new xBYTE[joinsBC];
            memcpy (joins, joinsBP, sizeof(xBYTE)*joinsBC);
            if (i < joinsBC)
                joins[i] = joinsBP[joinsBC];
            delete[] joinsBP;
            joinsBP = joins;
            break;
        }
}
void xIKNode :: JoinEDelete(xBYTE joinId)
{
    xBYTE *join = joinsEP;
    for (int i = 0; i < joinsEC; ++i, ++join)
        if (*join == joinId)
        {
            --joinsEC;
            xBYTE *joins = new xBYTE[joinsEC];
            memcpy (joins, joinsEP, sizeof(xBYTE)*joinsEC);
            if (i < joinsEC)
                joins[i] = joinsEP[joinsEC];
            delete[] joinsEP;
            joinsEP = joins;
            break;
        }
}
void xIKNode :: JoinBReplace(xBYTE oldId, xBYTE newId)
{
    xBYTE *join = joinsBP;
    for (int i = 0; i < joinsBC; ++i, ++join)
        if (*join == oldId)
        {
            *join = newId;
            break;
        }
}
void xIKNode :: JoinEReplace(xBYTE oldId, xBYTE newId)
{
    xBYTE *join = joinsEP;
    for (int i = 0; i < joinsEC; ++i, ++join)
        if (*join == oldId)
        {
            *join = newId;
            break;
        }
}

void xIKNode :: CloneTo(xIKNode &dst) const
{
    dst = *this;

    if (name) dst.name = strdup(name);

    if (joinsBC)
    {
        dst.joinsBP = new xBYTE[joinsBC];
        memcpy (dst.joinsBP, joinsBP, sizeof(xBYTE)*joinsBC);
    }

    if (joinsEC)
    {
        dst.joinsEP = new xBYTE[joinsEC];
        memcpy (dst.joinsEP, joinsEP, sizeof(xBYTE)*joinsEC);
    }
}

void xIKNode :: Load( FILE *file )
{
    xBYTE nameLen;
    fread(&nameLen, sizeof(xBYTE), 1, file);
    if (nameLen)
    {
        this->name = new char[nameLen];
        fread(this->name, 1, nameLen, file);
    }
    else
        this->name = NULL;

    fread(&this->id,      sizeof(xBYTE), 1, file);

    fread(&this->pointB,  sizeof(xVector3), 1, file);
    fread(&this->joinsBC, sizeof(xBYTE), 1, file);
    if (this->joinsBC)
    {
        this->joinsBP = new xBYTE[this->joinsBC];
        fread(this->joinsBP,  sizeof(xBYTE)*this->joinsBC, 1, file);
    }
    else
        this->joinsBP = NULL;
    fread(&this->pointE,  sizeof(xVector3), 1, file);
    fread(&this->joinsEC, sizeof(xBYTE), 1, file);
    if (this->joinsEC)
    {
        this->joinsEP = new xBYTE[this->joinsEC];
        fread(this->joinsEP,  sizeof(xBYTE)*this->joinsEC, 1, file);
    }
    else
        this->joinsEP = NULL;

    fread(&this->quaternion,  sizeof(xVector4), 1, file);
    fread(&this->curLengthSq, sizeof(xFLOAT), 1, file);
    fread(&this->minLengthSq, sizeof(xFLOAT), 1, file);
    fread(&this->maxLengthSq, sizeof(xFLOAT), 1, file);

    this->destination = this->pointE;
    this->forcesValid = false;
}
void xIKNode :: Save( FILE *file ) const
{
    xBYTE nameLen = 0;
    if (this->name)
        nameLen = strlen(this->name)+1;
    fwrite(&nameLen,      sizeof(xBYTE), 1, file);
    if (nameLen)
        fwrite(this->name, 1, nameLen, file);

    fwrite(&this->id,      sizeof(xBYTE), 1, file);

    fwrite(&this->pointB,  sizeof(xVector3), 1, file);
    fwrite(&this->joinsBC, sizeof(xBYTE), 1, file);
    fwrite(this->joinsBP,  sizeof(xBYTE)*this->joinsBC, 1, file);
    fwrite(&this->pointE,  sizeof(xVector3), 1, file);
    fwrite(&this->joinsEC, sizeof(xBYTE), 1, file);
    fwrite(this->joinsEP,  sizeof(xBYTE)*this->joinsEC, 1, file);

    fwrite(&this->quaternion,  sizeof(xVector4), 1, file);
    fwrite(&this->curLengthSq, sizeof(xFLOAT), 1, file);
    fwrite(&this->minLengthSq, sizeof(xFLOAT), 1, file);
    fwrite(&this->maxLengthSq, sizeof(xFLOAT), 1, file);
}
