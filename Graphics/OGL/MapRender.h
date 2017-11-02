#ifndef __incl_Graphics_OGL_MapRender_h
#define __incl_Graphics_OGL_MapRender_h

#include "../../World/Map.h"
#include "ogl.h"
#include "../Textures/TextureMgr.h"
#include "../../Math/Cameras/FieldOfView.h"

namespace Graphics { namespace OGL {
    using namespace World;

    class MapRender
    {
    private:
        // Birds
        GLint ListBirdHgh; // High LOD bird model
        GLint ListBirdMed; // Medium LOD bird model
        GLint ListBirdLow; // Low LOD bird model
        GLint ListBirdFar; // Lowes LOD bird model

        // Textures
        HTexture WallTexture;
        HTexture WallTexture_Normal;
        HTexture GroundTexture;
        HTexture GroundTexture_Normal;

        // Map Pointer
        const World::Map *Map;

        // Districts (building rendering batches)
        struct DistrictInfo
        {
            GLint ListNorth; // north wall batch
            GLint ListSouth; // soutch wall batch
            GLint ListWest;  // west wall batch
            GLint ListEast;  // east wall batch
            GLint ListTop;   // roof batch
            float maxHeight; // max height in batch (needed for clipping)
        }  *L_districts;
        int I_districts;     // number of districts

        void RenderDistrict(const Math::Cameras::FieldOfView &fov,

                                DistrictInfo &lists,    // batch data
                                int s_col, int s_row,   // col / row at which district starts
                                int s_cols, int s_rows, // no of cols / rows to render in batch

                                bool FL_showNorth, bool FL_showSouth, // which walls may be visible
                                bool FL_showWest,  bool FL_showEast,
                                bool FL_showTop);

    public:
        static float LOD_MedDistance_Sqr; // distance behind which we show Medium LOD model
        static float LOD_LowDistance_Sqr; // distance behind which we show Low LOD model
        static float LOD_FarDistance_Sqr; // distance behind which we show Far LOD model

        MapRender() { Clear(); }

        void Clear()
        {
            ListBirdHgh = ListBirdMed = ListBirdLow = ListBirdFar = 0;
            GroundTexture        = WallTexture        = HTexture();
            GroundTexture_Normal = WallTexture_Normal = HTexture();
            I_districts = 0;
            L_districts = NULL;
        }

        void Create(const World::Map &map)
        {
            Clear();
            Map = &map;
            I_districts = (((map.Cols-1) / Map::DistrictSize)+1) * (((map.Rows-1) / Map::DistrictSize) + 1);
            L_districts = new DistrictInfo[I_districts];
            memset(L_districts, 0, sizeof(DistrictInfo)*I_districts);
        }

        void Invalidate()
        {
            ListBirdHgh = ListBirdMed = ListBirdLow = ListBirdFar = 0;
            memset(L_districts, 0, sizeof(DistrictInfo)*I_districts);
        }

        void Destroy();

        void RenderBirds(const Math::Cameras::FieldOfView &fov);
        void RenderCity(const Math::Cameras::FieldOfView &fov);
    };

} } // namespace Graphics::OGL

#endif
