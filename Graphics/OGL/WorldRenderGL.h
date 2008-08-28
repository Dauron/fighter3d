#ifndef __incl_Graphics_OGL_WorldRenderGL_h
#define __incl_Graphics_OGL_WorldRenderGL_h

#include "Render/RendererGL.h"
#include "../../Math/Cameras/CameraSet.h"
#include "../../World/World.h"

class WorldRenderGL
{
public:
    RendererGL renderModel;

    void SetLight(xLight &light, bool t_Ambient, bool t_Diffuse, bool t_Specular);

    void RenderWorld(World &world, Math::Cameras::CameraSet &cameraSet);

    void RenderWorld(World &world, xLight &light, xColor sky, Math::Cameras::CameraSet &cameraSet);

    void RenderWorldNoLights(World &world, xColor sky, Math::Cameras::CameraSet &cameraSet);

    void InvalidateBones(RigidObj &obj)
    {
        if (obj.ModelGr) renderModel.InvalidateBonePositions(obj.ModelGr->instance);
        if (obj.ModelPh) renderModel.InvalidateBonePositions(obj.ModelPh->instance);
        obj.FL_renderNeedsUpdate = false;
        obj.FL_renderNeedsUpdateBones = false;
    }

    void Invalidate(RigidObj &obj)
    {
        if (obj.ModelGr) renderModel.InvalidateGraphics(*obj.ModelGr->xModelP, obj.ModelGr->instance);
        if (obj.ModelPh) renderModel.InvalidateGraphics(*obj.ModelPh->xModelP, obj.ModelPh->instance);
        obj.GetShadowMap().texId = 0;
        obj.FL_renderNeedsUpdate = false;
        obj.FL_renderNeedsUpdateBones = false;
    }

    void Invalidate(World &world)
    {
        if (world.skyBox) Invalidate(*world.skyBox);

        World::Vec_Object::iterator MD_curr = world.objects.begin(),
                                    MD_last = world.objects.end();

        for (; MD_curr != MD_last ; ++MD_curr )
            Invalidate(*(RigidObj*)*MD_curr);
    }

    void Free(RigidObj &obj)
    {
        if (obj.ModelGr) renderModel.FreeGraphics(*obj.ModelGr->xModelP, obj.ModelGr->instance, obj.ModelGr->GetReferences() == 1);
        if (obj.ModelPh) renderModel.FreeGraphics(*obj.ModelPh->xModelP, obj.ModelPh->instance, obj.ModelPh->GetReferences() == 1);

        xShadowMap &smap = obj.GetShadowMap();
        if (smap.texId)
        {
            GLuint tid = smap.texId;
            glDeleteTextures(1, &tid);
            smap.texId = 0;
        }
        obj.FL_renderNeedsUpdate = false;
        obj.FL_renderNeedsUpdateBones = false;
    }

    void FreeIfNeeded(World &world)
    {
        if (world.skyBox && world.skyBox->FL_renderNeedsUpdate) Free(*world.skyBox);

        World::Vec_Object::iterator MD_curr = world.objects.begin(),
                                    MD_last = world.objects.end();

        for (; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            if (obj.FL_renderNeedsUpdate) Free(obj);
            else
            if (obj.FL_renderNeedsUpdateBones) InvalidateBones(obj);
        }
    }

    void Free(World &world)
    {
        if (world.skyBox) Free(*world.skyBox);

        World::Vec_Object::iterator MD_curr = world.objects.begin(),
                                    MD_last = world.objects.end();

        for (; MD_curr != MD_last ; ++MD_curr )
            Free(*(RigidObj*)*MD_curr);
    }
};

#endif
