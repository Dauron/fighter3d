#ifndef __incl_Physics_Colliders_xBVHierarchy_h
#define __incl_Physics_Colliders_xBVHierarchy_h

#include "xIFigure3d.h"
#include "xBoxA.h"

namespace Math { namespace Figures {
    using namespace ::Math::Figures;

    struct xBVHierarchy {

    private:
        xIFigure3d    *FigureTransformed;
        bool           FL_RawToLocal;
        xMatrix        MX_RawToLocal;
        xMatrix        MX_LocalToWorld;
        xMatrix        MX_RawToWorld;

    public:
        union {
            xBYTE         ID_Element;
            xBYTE         ID_Bone;
        };

        xIFigure3d   *Figure;
        xIFigure3d   *GetTransformed(const xMatrix &mx_LocalToWorld)
        {
            if (!FigureTransformed)
            {
                MX_LocalToWorld = mx_LocalToWorld;
                if (FL_RawToLocal)
                    FigureTransformed = Figure->Transform(MX_RawToWorld = MX_RawToLocal * mx_LocalToWorld);
                else
                    FigureTransformed = Figure->Transform(MX_RawToWorld = mx_LocalToWorld);
            }
            return FigureTransformed;
        }
        xIFigure3d   *GetTransformed()
        {
            return GetTransformed(MX_RawToLocal_Get());
        }
        bool FL_RawToLocal_Get()
        { return FL_RawToLocal; }
        void MX_RawToLocal_Set(const xMatrix &mx_RawToLocal)
        {
            if (mx_RawToLocal.isIdentity())
                FL_RawToLocal = false;
            else
            {
                FL_RawToLocal = true;
                MX_RawToLocal = mx_RawToLocal;
            }
        }
        const xMatrix &MX_RawToLocal_Get() const
        {
            if (FL_RawToLocal)
                return MX_RawToLocal;
            return xMatrix::Identity();
        }
        const xMatrix &MX_LocalToWorld_Get() const
        {
            return MX_LocalToWorld;
        }
        const xMatrix &MX_RawToWorld_Get() const
        {
            return MX_RawToWorld;
        }

        xBYTE         I_items;
        xBVHierarchy *L_items;

        void init(xIFigure3d &figure)
        {
            I_items = 0;
            L_items = NULL;
            Figure  = &figure;
            FigureTransformed = NULL;
            FL_RawToLocal = false;
            MX_LocalToWorld.identity();
            MX_RawToWorld.identity();
        }

        void zero()
        {
            I_items = 0;
            L_items = NULL;
            Figure  = NULL;
            FigureTransformed = NULL;
            FL_RawToLocal = false;
            MX_LocalToWorld.identity();
            MX_RawToWorld.identity();
        }

        void invalidateTransformation()
        {
            if (FigureTransformed)
            {
                FigureTransformed->free(true);
                delete FigureTransformed;
                FigureTransformed = NULL;

                if (I_items)
                {
                    xBVHierarchy *iter = L_items;
                    for (int i = I_items; i; --i, ++iter) iter->invalidateTransformation();
                }
            }
        }

        void free()
        {
            if (Figure) { Figure->free(); delete Figure; Figure = NULL; }
            if (I_items)
            {
                xBVHierarchy *iter = L_items;
                for (int i = I_items; i; --i, ++iter) iter->free();
                delete[] (xBYTE*) L_items;
                I_items = 0;
            }
        }

        void Clone(xBVHierarchy &res)
        {
            res.I_items = I_items;
            res.L_items = new xBVHierarchy[I_items];

            res.ID_Bone           = ID_Bone;
            res.FigureTransformed = NULL;
            res.FL_RawToLocal     = FL_RawToLocal;
            res.MX_RawToLocal     = MX_RawToLocal;
            res.MX_LocalToWorld   = MX_LocalToWorld;
            res.MX_RawToWorld     = MX_RawToWorld;

            res.Figure            = Figure->Transform(xMatrix::Identity());

            for (int i = 0; i < I_items; ++i)
                L_items[i].Clone(res.L_items[i]);
        }
        void Save( FILE *file )
        {
            fwrite(&ID_Bone,       sizeof(ID_Bone), 1, file);
            fwrite(&FL_RawToLocal, sizeof(FL_RawToLocal), 1, file);
            fwrite(&MX_RawToLocal, sizeof(MX_RawToLocal), 1, file);
            Figure->Save(file);
            fwrite(&I_items,       sizeof(ID_Bone), 1, file);
            for (int i = 0; i < I_items; ++i)
                L_items[i].Save(file);
        }
        void Load( FILE *file )
        {
            fread(&ID_Bone,       sizeof(ID_Bone), 1, file);
            fread(&FL_RawToLocal, sizeof(FL_RawToLocal), 1, file);
            fread(&MX_RawToLocal, sizeof(MX_RawToLocal), 1, file);
            Figure = xIFigure3d::Load(file);
            fread(&I_items,       sizeof(ID_Bone), 1, file);
            L_items = new xBVHierarchy[I_items];
            for (int i = 0; i < I_items; ++i)
                L_items[i].Load(file);

            FigureTransformed = NULL;
            MX_LocalToWorld.identity();
            MX_RawToWorld.identity();
        }

        xBVHierarchy *add (xIFigure3d &figure)
        {
            xBVHierarchy *L_temp = (xBVHierarchy *) new xBYTE[(I_items+1) * sizeof(xBVHierarchy)];
            if (I_items)
            {
                memcpy(L_temp, L_items, I_items*sizeof(xBVHierarchy));
                delete[] (xBYTE*) L_items;
            }
            L_items = L_temp;
            L_items[I_items].init(figure);
            ++I_items;
            return L_items + I_items-1;
        }

        void remove (xBYTE I_idx)
        {
            if (I_idx >= I_items) return;

            L_items[I_idx].free();

            if (I_items == 1)
            {
                delete[] (xBYTE*) L_items;
                L_items = NULL;
                I_items = 0;
                return;
            }

            xBVHierarchy *L_temp = (xBVHierarchy *) new xBYTE[(--I_items) * sizeof(xBVHierarchy)];
            if (I_idx > 0)       memcpy(L_temp, L_items, I_idx*sizeof(xBVHierarchy));
            if (I_idx < I_items) memcpy(L_temp+I_idx, L_items+(I_idx+1), (I_items-I_idx)*sizeof(xBVHierarchy));
            delete[] (xBYTE*) L_items;
            L_items = L_temp;
        }

        bool remove (xBVHierarchy *bvhnode)
        {
            if (bvhnode >= L_items && bvhnode < L_items + I_items)
            {
                remove ( bvhnode - L_items );
                return true;
            }

            for (int i = 0; i < I_items; ++i)
                if (L_items[i].remove(bvhnode))
                    return true;

            return false;
        }

        xBoxA childBounds()
        {
            xBoxA box;

            if (I_items)
            {
                xIFigure3d *figure = L_items[0].FL_RawToLocal_Get()
                    ? L_items[0].GetTransformed()
                    : L_items[0].Figure;
                figure->P_MinMax_Get(box.P_min, box.P_max);

                xPoint3 P_min, P_max;
                for (int i = 1; i < I_items; ++i)
                {
                    figure = L_items[i].FL_RawToLocal_Get()
                        ? L_items[i].GetTransformed()
                        : L_items[i].Figure;
                    figure->P_MinMax_Get(P_min, P_max);

                    if (P_min.x < box.P_min.x) { box.P_min.x = P_min.x; }
                    if (P_max.x > box.P_max.x) { box.P_max.x = P_max.x; }
                    if (P_min.y < box.P_min.y) { box.P_min.y = P_min.y; }
                    if (P_max.y > box.P_max.y) { box.P_max.y = P_max.y; }
                    if (P_min.z < box.P_min.z) { box.P_min.z = P_min.z; }
                    if (P_max.z > box.P_max.z) { box.P_max.z = P_max.z; }
                }
            }
            else
            {
                box.P_min.init(0,0,0);
                box.P_max.init(0,0,0);
            }
            return box;
        }

        xBoxA childBounds(const xMatrix &matrix)
        {
            xBoxA box;

            if (I_items)
            {
                xIFigure3d *figure = L_items[0].GetTransformed(matrix);
                figure->P_MinMax_Get(box.P_min, box.P_max);

                xPoint3 P_min, P_max;
                for (int i = 1; i < I_items; ++i)
                {
                    figure = L_items[i].GetTransformed(matrix);
                    figure->P_MinMax_Get(P_min, P_max);

                    if (P_min.x < box.P_min.x) { box.P_min.x = P_min.x; }
                    if (P_max.x > box.P_max.x) { box.P_max.x = P_max.x; }
                    if (P_min.y < box.P_min.y) { box.P_min.y = P_min.y; }
                    if (P_max.y > box.P_max.y) { box.P_max.y = P_max.y; }
                    if (P_min.z < box.P_min.z) { box.P_min.z = P_min.z; }
                    if (P_max.z > box.P_max.z) { box.P_max.z = P_max.z; }
                }
            }
            else
            {
                box.P_min.init(0,0,0);
                box.P_max.init(0,0,0);
            }
            return box;
        }
    };

} } // namespace Math.Figures

#endif
