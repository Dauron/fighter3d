#ifndef __incl_World_h
#define __incl_World_h

#include "../Physics/PhysicalWorld.h"
#include "RigidObj.h"

class World : public Physics::PhysicalWorld
{
public:
    Vec_Object objects;
    Vec_xLight lights;

    RigidObj *skyBox;
    xColor    skyColor;

    xMatrix   MX_spawn1;
    xMatrix   MX_spawn2;

public:

    World( void ) : skyBox(NULL) { skyColor.init(0.f,0.f,0.f,0.f); }

    void Initialize (std::string MapFileName);
    void Finalize   ();

    void InitialUpdate()
    {
        if (skyBox) skyBox->FrameUpdate(0.f);
        Vec_Object::iterator model, beginM = objects.begin(), endM = objects.end();
        for (model=beginM; model!=endM; ++model)
            (**model).FrameUpdate(0.f);

        Vec_xLight::iterator LT_curr = lights.begin(),
                             LT_last = lights.end();
        for (; LT_curr != LT_last ; ++LT_curr)
            LT_curr->update();
    }

    void FrameStart()
    {
        if (skyBox) skyBox->FrameStart();
        Vec_Object::iterator model, beginM = objects.begin(), endM = objects.end();
        for (model=beginM; model!=endM; ++model)
            (**model).FrameStart();
    }
    void FrameUpdate (float T_delta);
    void FrameRender()
    {
        if (skyBox) skyBox->FrameRender();
        Vec_Object::iterator model, beginM = objects.begin(), endM = objects.end();
        for (model=beginM; model!=endM; ++model)
            (**model).FrameRender();
    }
    void FrameEnd()
    {
        if (skyBox) skyBox->FrameEnd();
        Vec_Object::iterator model, beginM = objects.begin(), endM = objects.end();
        for (model=beginM; model!=endM; ++model)
            (**model).FrameEnd();
    }

    void Load       (const char *mapFileName);
};

#endif
