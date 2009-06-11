#ifndef __incl_Models_Solids_Plane_h
#define __incl_Models_Solids_Plane_h

#include "../lib3dx/xModel.h"
#include "HeightMap.h"

namespace Models { namespace Solids {
    using namespace Models::Solids;

    class Plane {
    public:
        static xModel *GeneratePlane(HeightMap map, float dist, float max_height)
        {
            assert(map.Heights && "HeightMap is not created!");

            max_height /= 255;

            xModel *model = new xModel();
            model->BVHierarchy = NULL;
            model->FileName    = NULL;

            model->I_material = 1;
            model->L_material = new xMaterial();
            memset(model->L_material, 0, sizeof(xMaterial));
            model->L_material->Name = strdup("mat_Plane");
            model->L_material->ambient.init(0.4f, 0.4f, 1.f, 1.f);
            model->L_material->diffuse.init(0.8f, 0.8f, 1.f, 1.f);
            model->L_material->specular.init(0.8f, 0.8f, 1.f, 1.f);

            //////////////////// VERTS
            xElement *plane = new xElement;
            plane->ID    = 0;
            plane->Name  = strdup("Plane");
            plane->Color.init(0.8f, 1.f, 0.8f, 1.f);
            plane->MX_MeshToLocal.identity();
            plane->L_colors = NULL;

            plane->I_vertices = map.Size;
            plane->L_verticesT = new xVertexTex[plane->I_vertices];

            xVertexTex *VT_curr = plane->L_verticesT;
            xBYTE      *HT_curr = map.Heights;
            xFLOAT y = 0;
            for (int r = 0; r <= map.Rows; ++r, y += dist)
            {
                xFLOAT x = 0;
                for (int c = 0; c <= map.Cols; ++c, ++VT_curr, ++HT_curr, x += dist)
                {
                    VT_curr->pos.init(x, y, (*HT_curr + map.BaseHeight) * max_height);
                    VT_curr->tex.init(c / (float)map.Cols, r / (float)map.Rows);
                }
            }

            ///////////////// COLORS

            plane->L_colors = new xBYTE3[map.Size];
            xBYTE3 *PX_curr = plane->L_colors;
            xBYTE3 *PX_last = PX_curr + map.Size;
            HT_curr = map.Heights;

            xBYTE CL_water_low [3] = {0,  0,  128};
            xBYTE CL_water_high[3] = {150,150,255};
            xBYTE CL_green_low [3] = {0,  180,  0};
            xBYTE CL_green_high[3] = {0,  120,  0};
            xBYTE CL_gray_low  [3] = {120,120,120};
            xBYTE CL_gray_high [3] = {255,255,255};

            for (; PX_curr < PX_last; ++PX_curr, ++HT_curr)
            {
                xWORD height = *HT_curr + map.BaseHeight;
                if (height <= 100)
                {
                    xFLOAT scale = height / 100.f;
                    xFLOAT compl = 1.f - scale;
                    (*PX_curr)[0] = (xBYTE) (scale * CL_water_high[0] + compl * CL_water_low[0]);
                    (*PX_curr)[1] = (xBYTE) (scale * CL_water_high[1] + compl * CL_water_low[1]);
                    (*PX_curr)[2] = (xBYTE) (scale * CL_water_high[2] + compl * CL_water_low[2]);
                }
                else
                if (height < 180)
                {
                    xFLOAT scale = (height-100) / 80.f;
                    xFLOAT compl = 1.f - scale;
                    (*PX_curr)[0] = (xBYTE) (scale * CL_green_high[0] + compl * CL_green_low[0]);
                    (*PX_curr)[1] = (xBYTE) (scale * CL_green_high[1] + compl * CL_green_low[1]);
                    (*PX_curr)[2] = (xBYTE) (scale * CL_green_high[2] + compl * CL_green_low[2]);
                }
                else
                if (height < 240)
                {
                    xFLOAT scale = (height-180) / 60.f;
                    xFLOAT compl = 1.f - scale;
                    (*PX_curr)[0] = (xBYTE) (compl * CL_green_high[0] + scale * CL_gray_low[0]);
                    (*PX_curr)[1] = (xBYTE) (compl * CL_green_high[1] + scale * CL_gray_low[1]);
                    (*PX_curr)[2] = (xBYTE) (compl * CL_green_high[2] + scale * CL_gray_low[2]);
                }
                else
                {
                    xFLOAT scale = height < 255.f ? (height-240) / 15.f : 1.f;
                    xFLOAT compl = 1.f - scale;
                    (*PX_curr)[0] = (xBYTE) (scale * CL_gray_high[0] + compl * CL_gray_low[0]);
                    (*PX_curr)[1] = (xBYTE) (scale * CL_gray_high[1] + compl * CL_gray_low[1]);
                    (*PX_curr)[2] = (xBYTE) (scale * CL_gray_high[2] + compl * CL_gray_low[2]);
                }
            }

            ///////////////// FACES

            plane->I_faces  = map.Rows * map.Cols * 2;
            plane->L_faces  = new xFace[plane->I_faces];
            plane->L_smooth = new xDWORD[plane->I_faces];

            xFace  *F_iter = plane->L_faces;
            xDWORD *S_iter = plane->L_smooth;

            int id = 0, id_n = map.Cols+1;
            for (int r = 0; r < map.Rows; ++r)
            {
                for (int c = 0; c < map.Cols; ++c, ++VT_curr)
                {
                    (*F_iter)[0] = id;
                    (*F_iter)[1] = ++id;
                    (*F_iter)[2] = id_n;
                    *(S_iter++) = 1;
                    ++F_iter;

                    (*F_iter)[1] = id;
                    (*F_iter)[0] = id_n;
                    (*F_iter)[2] = ++id_n;
                    *(S_iter++) = 1;
                    ++F_iter;
                }
                ++id;
                ++id_n;
            }

            plane->I_faceLists = 1;
            plane->L_faceLists = new xFaceList[1];
            plane->L_faceLists->FL_smooth = true;
            plane->L_faceLists->I_offset  = 0;
            plane->L_faceLists->I_count   = plane->I_faces;
            plane->L_faceLists->ID_material = 0;
            plane->L_faceLists->Material    = NULL;//model->L_material;

            plane->L_edges = NULL;
            plane->I_edges = 0;

            plane->FL_textured    = true;
            plane->FL_skeletized  = false;
            plane->FL_transparent = false;
            plane->FL_opaque      = true;

            plane->Next   = NULL;
            plane->L_kids = NULL;
            plane->I_kids = 0;

            memset(&plane->boundingData, 0, sizeof(plane->boundingData));
            memset(&plane->renderData,   0, sizeof(plane->renderData));
            plane->CalculateSmoothVertices(false);

            ////////////////////
            model->L_kids     = plane;
            model->I_kids     = 1;
            model->I_elements = 1;

            memset(&model->Spine, 0, sizeof(model->Spine));

            model->FL_textures_loaded = false;
            model->FL_save_rdata      = false;
            model->FL_save_binfo      = false;
            model->FL_transparent     = false;
            model->FL_opaque          = true;

            return model;
        }
    };

} }

#endif
