#ifndef __incl_World_h
#define __incl_World_h

#include "SkeletizedObj.h"
#include "../Physics/PhysicalWorld.h"

class World : public Physics::PhysicalWorld
{
public:
    Vec_Object objects;
    Vec_xLight lights;

    RigidObj *skyBox;

public:

    World( void ) : skyBox(NULL) {}

    void Initialize ();
    void Finalize   ();

    void Invalidate()
    {
        Vec_xLight::iterator light, begin = lights.begin(), end = lights.end();
        for (light=begin; light!=end; ++light)
            light->modified = true;

        Vec_Object::iterator model, beginM = objects.begin(), endM = objects.end();
        for (model=beginM; model!=endM; ++model)
            (*model)->Invalidate();
    }

    void InitialUpdate()
    {
        Vec_Object::iterator model, beginM = objects.begin(), endM = objects.end();
        for (model=beginM; model!=endM; ++model)
            (*model)->FrameUpdate(0.f);
    }

    void FrameStart()
    {
        Vec_Object::iterator model, beginM = objects.begin(), endM = objects.end();
        for (model=beginM; model!=endM; ++model)
            (*model)->FrameStart();
    }
    void FrameUpdate (float T_delta);
    void FrameRender()
    {
        Vec_Object::iterator model, beginM = objects.begin(), endM = objects.end();
        for (model=beginM; model!=endM; ++model)
            (*model)->FrameRender();
    }
    void FrameEnd()
    {
        Vec_Object::iterator model, beginM = objects.begin(), endM = objects.end();
        for (model=beginM; model!=endM; ++model)
            (*model)->FrameEnd();
    }

    void Load       (const char *mapFileName);

    ~World( void )
    {
        Finalize();
    }
};

#endif
