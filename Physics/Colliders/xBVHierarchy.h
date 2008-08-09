#ifndef __incl_Physics_xBVHierarchy_h
#define __incl_Physics_xBVHierarchy_h

#include "../../Math/Figures/xIFigure3d.h"

namespace Math { namespace Figures {
    using namespace ::Math::Figures;

    struct xBVHierarchy {

    private:
        xIFigure3d   *FigureTransformed;

    public:

        xIFigure3d   *Figure;
        xIFigure3d   *GetTransformed(const xMatrix &MX_FigToWorld)
        {
            if (!FigureTransformed)
                FigureTransformed = Figure->Transform(MX_FigToWorld);
            return FigureTransformed;
        }

        xBYTE         I_items;
        xBVHierarchy *L_items;

        void init(xIFigure3d *figure)
        {
            I_items = 0;
            L_items = NULL;
            Figure  = figure;
            FigureTransformed = NULL;
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
            Figure->free();
            if (Figure)  delete Figure;
            if (I_items)
            {
                xBVHierarchy *iter = L_items;
                for (int i = I_items; i; --i, ++iter) iter->free();
                delete[] (xBYTE*) L_items;
            }
        }

        void add (xIFigure3d *figure)
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
        }

        void remove (xBYTE I_idx)
        {
            if (I_idx < 0 || I_idx >= I_items) return;

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
            if (I_idx < I_items) memcpy(L_temp+I_idx, L_items+(I_idx+1), (I_items-I_idx-1)*sizeof(xBVHierarchy));
            delete[] (xBYTE*) L_items;
            L_items = L_temp;
        }
    };

} } // namespace Math.Figures

#endif
