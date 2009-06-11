#ifndef __incl_World_h
#define __incl_World_h

#include "../Physics/PhysicalWorld.h"
#include "RigidObj.h"
#include "../Models/Solids/Dome.h"
#include "../Models/Particles/Engine.h"

class World : public Physics::PhysicalWorld
{
public:
    Vec_Object objects;
    Vec_xLight lights;

    Models::Particles::
    CEngine         particleEngine;

    Models::Solids::
    CDome           skyDome;
    xPoint3         P_Sun;
    xColor3b        P_SunColor;

    RigidObj *skyBox;
    xColor    skyColor;

    xMatrix   MX_spawn1;
    xMatrix   MX_spawn2;

public:

    World( void ) : skyBox(NULL) { skyColor.init(0.f,0.f,0.f,0.f); }

    void Create  (std::string MapFileName);
    void Destroy ();

    void InitialUpdate()
    {
        if (skyBox) skyBox->Update(0.f);
        Vec_Object::iterator model, beginM = objects.begin(), endM = objects.end();
        for (model=beginM; model!=endM; ++model)
            (**model).Update(0.f);

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
    void Update (float T_delta);
    void Render()
    {
        if (skyBox) skyBox->Render();
        Vec_Object::iterator model, beginM = objects.begin(), endM = objects.end();
        for (model=beginM; model!=endM; ++model)
            (**model).Render();
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
