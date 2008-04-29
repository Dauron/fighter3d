/* -*- c -*- */
#ifndef INCLUDED_LIB3DS_FILE_H
#define INCLUDED_LIB3DS_FILE_H
/*
 * The 3D Studio File Format Library
 * Copyright (C) 1996-2007 by Jan Eric Kyprianidis <www.kyprianidis.com>
 * All rights reserved.
 *
 * This program is  free  software;  you can redistribute it and/or modify it
 * under the terms of the  GNU Lesser General Public License  as published by
 * the  Free Software Foundation;  either version 2.1 of the License,  or (at
 * your option) any later version.
 *
 * This  program  is  distributed in  the  hope that it will  be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public
 * License for more details.
 *
 * You should  have received  a copy of the GNU Lesser General Public License
 * along with  this program;  if not, write to the  Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: file.h,v 1.24 2007/06/20 17:04:08 jeh Exp $
 */

#ifndef INCLUDED_LIB3DS_BACKGROUND_H
#include "background.h"
#endif
#ifndef INCLUDED_LIB3DS_ATMOSPHERE_H
#include "atmosphere.h"
#endif
#ifndef INCLUDED_LIB3DS_SHADOW_H
#include "shadow.h"
#endif
#ifndef INCLUDED_LIB3DS_VIEWPORT_H
#include "viewport.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 3DS file structure
 * \ingroup file
 */
struct Lib3dsFile {
    Lib3dsDword mesh_version;
    Lib3dsWord keyf_revision;
    char name[12+1];
    Lib3dsFloat master_scale;
    Lib3dsVector construction_plane;
    Lib3dsRgb ambient;
    Lib3dsShadow shadow;
    Lib3dsBackground background;
    Lib3dsAtmosphere atmosphere;
    Lib3dsViewport viewport;
    Lib3dsViewport viewport_keyf;
    Lib3dsIntd frames;
    Lib3dsIntd segment_from;
    Lib3dsIntd segment_to;
    Lib3dsIntd current_frame;
    Lib3dsMaterial *materials;
    Lib3dsMesh *meshes;
    Lib3dsCamera *cameras;
    Lib3dsLight *lights;
    Lib3dsNode *nodes;
};

Lib3dsFile* lib3ds_file_load(const char *filename);
Lib3dsBool lib3ds_file_save(Lib3dsFile *file, const char *filename);
Lib3dsFile* lib3ds_file_new();
void lib3ds_file_free(Lib3dsFile *file);
void lib3ds_file_eval(Lib3dsFile *file, Lib3dsFloat t);
Lib3dsBool lib3ds_file_read(Lib3dsFile *file, Lib3dsIo *io);
Lib3dsBool lib3ds_file_write(Lib3dsFile *file, Lib3dsIo *io);
void lib3ds_file_insert_material(Lib3dsFile *file, Lib3dsMaterial *material);
void lib3ds_file_remove_material(Lib3dsFile *file, Lib3dsMaterial *material);
Lib3dsMaterial* lib3ds_file_material_by_name(Lib3dsFile *file, const char *name);
void lib3ds_file_dump_materials(Lib3dsFile *file);
void lib3ds_file_insert_mesh(Lib3dsFile *file, Lib3dsMesh *mesh);
void lib3ds_file_remove_mesh(Lib3dsFile *file, Lib3dsMesh *mesh);
Lib3dsMesh* lib3ds_file_mesh_by_name(Lib3dsFile *file, const char *name);
void lib3ds_file_dump_meshes(Lib3dsFile *file);
void lib3ds_file_dump_instances(Lib3dsFile *file);
void lib3ds_file_insert_camera(Lib3dsFile *file, Lib3dsCamera *camera);
void lib3ds_file_remove_camera(Lib3dsFile *file, Lib3dsCamera *camera);
Lib3dsCamera* lib3ds_file_camera_by_name(Lib3dsFile *file, const char *name);
void lib3ds_file_dump_cameras(Lib3dsFile *file);
void lib3ds_file_insert_light(Lib3dsFile *file, Lib3dsLight *light);
void lib3ds_file_remove_light(Lib3dsFile *file, Lib3dsLight *light);
Lib3dsLight* lib3ds_file_light_by_name(Lib3dsFile *file, const char *name);
void lib3ds_file_dump_lights(Lib3dsFile *file);
Lib3dsNode* lib3ds_file_node_by_name(Lib3dsFile *file, const char* name, Lib3dsNodeTypes type);
Lib3dsNode* lib3ds_file_node_by_id(Lib3dsFile *file, Lib3dsWord node_id);
void lib3ds_file_insert_node(Lib3dsFile *file, Lib3dsNode *node);
Lib3dsBool lib3ds_file_remove_node(Lib3dsFile *file, Lib3dsNode *node);
void lib3ds_file_bounding_box_of_objects(Lib3dsFile *file, Lib3dsBool include_meshes, Lib3dsBool include_cameras, Lib3dsBool include_lights, Lib3dsVector bmin, Lib3dsVector bmax);
void lib3ds_file_bounding_box_of_nodes(Lib3dsFile *file, Lib3dsBool include_meshes, Lib3dsBool include_cameras, Lib3dsBool include_lights, Lib3dsVector bmin, Lib3dsVector bmax);
void lib3ds_file_dump_nodes(Lib3dsFile *file);

#ifdef __cplusplus
}
#endif
#endif

