#include "xBone.h"

void xBone :: KidAdd (xBYTE ID_newKid)
{
    xBYTE *ID_newKids = new xBYTE[I_kids+1];
    memcpy (ID_newKids, ID_kids, sizeof(xBYTE)*I_kids);
    ID_newKids[I_kids] = ID_newKid;

    delete[] ID_kids;
    ID_kids = ID_newKids;
    ++I_kids;
}
void xBone :: KidDelete (xBYTE ID_delKid)
{
    xBYTE *ID_iter = ID_kids;
    for (int i = 0; i < I_kids; ++i, ++ID_iter)
        if (*ID_iter == ID_delKid)
        {
            --I_kids;
            if (I_kids)
            {
                xBYTE *ID_newKids = new xBYTE[I_kids];
                memcpy (ID_newKids, ID_kids, sizeof(xBYTE)*I_kids);
                if (i < I_kids) // move last to the position of the deleted one
                    ID_newKids[i] = ID_kids[I_kids];
                delete[] ID_kids;
                ID_kids = ID_newKids;
            }
            else
            {
                delete[] ID_kids;
                ID_kids = NULL;
            }
            break;
        }
}
void xBone :: KidReplace(xBYTE ID_oldKid, xBYTE ID_newKid)
{
    xBYTE *ID_iter = ID_kids;
    for (int i = I_kids; i ; --i, ++ID_iter)
        if (*ID_iter == ID_oldKid)
        {
            *ID_iter = ID_newKid;
            break;
        }
}

void xBone :: CloneTo(xBone &dst) const
{
    memcpy (&dst, this, sizeof(xBone));
    dst = *this;

    if (Name) dst.Name = strdup(Name);

    if (ID_kids)
    {
        dst.ID_kids = new xBYTE[I_kids];
        memcpy (dst.ID_kids, ID_kids, sizeof(xBYTE)*I_kids);
    }
}

void xBone :: Load( FILE *file )
{
    xBYTE I_nameLen;
    fread(&I_nameLen, sizeof(xBYTE), 1, file);
    if (I_nameLen)
    {
        this->Name = new char[I_nameLen];
        fread(this->Name, 1, I_nameLen, file);
    }
    else
        this->Name = NULL;

    fread(&this->ID,        sizeof(xBYTE), 1, file);
    fread(&this->ID_parent, sizeof(xBYTE), 1, file);

    fread(&this->M_weight,  sizeof(xFLOAT), 1, file);
    fread(&this->P_begin,   sizeof(xVector3), 1, file);
    fread(&this->P_end,     sizeof(xVector3), 1, file);

    fread(&this->I_kids,    sizeof(xBYTE), 1, file);
    if (this->I_kids)
    {
        this->ID_kids = new xBYTE[this->I_kids];
        fread(this->ID_kids, sizeof(xBYTE)*this->I_kids, 1, file);
    }
    else
        this->ID_kids = NULL;

    this->QT_rotation.zeroQ();
    this->S_lengthSqr = (P_end - P_begin).lengthSqr();
    this->S_length    = sqrt(this->S_lengthSqr);
}
void xBone :: Save( FILE *file ) const
{
    xBYTE I_nameLen = 0;
    if (this->Name)
        I_nameLen = strlen(this->Name)+1;
    fwrite(&I_nameLen, sizeof(xBYTE), 1, file);
    if (I_nameLen)
        fwrite(this->Name, 1, I_nameLen, file);

    fwrite(&this->ID,        sizeof(xBYTE), 1, file);
    fwrite(&this->ID_parent, sizeof(xBYTE), 1, file);

    fwrite(&this->M_weight,  sizeof(xFLOAT), 1, file);
    fwrite(&this->P_begin,   sizeof(xVector3), 1, file);
    fwrite(&this->P_end,     sizeof(xVector3), 1, file);

    fwrite(&this->I_kids,    sizeof(xBYTE), 1, file);
    if (this->I_kids)
        fwrite(this->ID_kids, sizeof(xBYTE)*this->I_kids, 1, file);
}
