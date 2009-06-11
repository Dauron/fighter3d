#ifndef __incl_Models_Solids_Sphere_h
#define __incl_Models_Solids_Sphere_h

#include "../lib3dx/xModel.h"

namespace Models { namespace Solids {

    class Sphere {
    public:
        static xModel *GenerateGlobe()
        {
            xModel *model = new xModel();
            model->BVHierarchy = NULL;
            model->FileName    = NULL;

            model->I_material = 1;
            model->L_material = new xMaterial();
            memset(model->L_material, 0, sizeof(xMaterial));
            model->L_material->Name = strdup("mat_Globe");
            model->L_material->ambient.init(0.4f, 0.4f, 1.f, 1.f);
            model->L_material->diffuse.init(0.8f, 0.8f, 1.f, 1.f);
            model->L_material->specular.init(0.8f, 0.8f, 1.f, 1.f);

            ////////////////////
            xElement *globe = new xElement;
            globe->ID    = 0;
            globe->Name  = strdup("Globe");
            globe->Color.init(0.8f, 0.8f, 1.f, 1.f);
            globe->MX_MeshToLocal.identity();
            globe->L_colors = NULL;

            globe->I_vertices = 37 * 19;
            globe->L_verticesT = new xVertexTex[globe->I_vertices];

            xWORD lastVert = globe->I_vertices - 1;

            xFLOAT zMax = 10000;
            xFLOAT zMax_2 = zMax * zMax;

            xVertexTex *GPS_iter = globe->L_verticesT;
            for (int longitude = 0; longitude <= 360; longitude += 10, ++GPS_iter)
            {
                GPS_iter->pos.init(0,0, zMax);
                GPS_iter->tex.x = longitude / 360.f;
                GPS_iter->tex.y = 0.999f;
            }
            for (int latitude = 80; latitude >= -80; latitude -= 10)
            {
                xFLOAT sinLat = sin(DegToRad(latitude));
                xFLOAT z = zMax * sinLat;
                xFLOAT x = sqrt(zMax_2-z*z);
                xFLOAT x_2 = x * x;

                xFLOAT t_y = sinLat * 0.5f + 0.5f;

                for (int longitude = 0; longitude < 360; longitude += 10, ++GPS_iter)
                {
                    GPS_iter->pos.z = z;
                    GPS_iter->pos.x = x*cos(DegToRad(longitude));
                    GPS_iter->pos.y = x*sin(DegToRad(longitude));
                    GPS_iter->tex.x = longitude / 360.f;
                    GPS_iter->tex.y = t_y;
                }

                GPS_iter->pos = (GPS_iter-36)->pos;
                GPS_iter->tex.x = 1.f;
                GPS_iter->tex.y = t_y;
                ++GPS_iter;
            }
            for (int longitude = 0; longitude <= 360; longitude += 10, ++GPS_iter)
            {
                GPS_iter->pos.init(0,0, -zMax);
                GPS_iter->tex.x = longitude / 360.f;
                GPS_iter->tex.y = 0.001f;
            }

            globe->I_faces  = 36 * 18 * 2;
            globe->L_faces  = new xFace[globe->I_faces];
            globe->L_smooth = new xDWORD[globe->I_faces];

            xFace  *F_iter = globe->L_faces;
            xDWORD *S_iter = globe->L_smooth;
            int longitude;

            F_iter = globe->L_faces;
            S_iter = globe->L_smooth;
            longitude = 37;
            for (int latitude = 80; latitude >= -90; latitude -= 10)
            {
                int max_lon = longitude + 36;
                for (; longitude < max_lon;)
                {
                    (*F_iter)[0] = longitude-37;
                    (*F_iter)[1] = longitude;
                    (*F_iter)[2] = longitude+1;
                    *(S_iter++) = 1;
                    ++F_iter;

                    (*F_iter)[0] = longitude-36;
                    (*F_iter)[1] = longitude-37;
                    (*F_iter)[2] = longitude+1;
                    *(S_iter++) = 1;
                    ++F_iter;

                    ++longitude;
                }
                ++longitude;
            }

            globe->I_faceLists = 1;
            globe->L_faceLists = new xFaceList[1];
            globe->L_faceLists->FL_smooth = true;
            globe->L_faceLists->I_offset  = 0;
            globe->L_faceLists->I_count   = globe->I_faces;
            globe->L_faceLists->ID_material = 0;
            globe->L_faceLists->Material    = model->L_material;

            globe->L_edges = NULL;
            globe->I_edges = 0;

            globe->FL_textured    = true;
            globe->FL_skeletized  = false;
            globe->FL_transparent = false;
            globe->FL_opaque      = true;

            globe->Next   = NULL;
            globe->L_kids = NULL;
            globe->I_kids = 0;

            memset(&globe->boundingData, 0, sizeof(globe->boundingData));
            memset(&globe->renderData,   0, sizeof(globe->renderData));
            globe->CalculateSmoothVertices();

            ////////////////////
            model->L_kids     = globe;
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