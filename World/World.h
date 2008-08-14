#ifndef __incl_World_h
#define __incl_World_h

#include "SkeletizedObj.h"
#include "../Physics/CD_MeshToMesh.h"
#include "../Physics/CD_RayToMesh.h"
#include "../Physics/PhysicalWorld.h"

class World : public Physics::PhysicalWorld
{
public:
    ObjectVector objects;
    xLightVector lights;

    RigidObj *skyBox;

public:

    World( void ) : skyBox(NULL) {}

    //RigidObj *CollideWithRay(xVector3 rayPos, xVector3 rayDir);

    void Initialize ();
    void Update     (float T_delta);
    void Finalize   ();

    void Invalidate()
    {
        xLightVector::iterator light, begin = lights.begin(), end = lights.end();
        for (light=begin; light!=end; ++light)
            light->modified = true;

        ObjectVector::iterator model, beginM = objects.begin(), endM = objects.end();
        for (model=beginM; model!=endM; ++model)
            (*model)->Invalidate();
    }

    void Load       (const char *mapFileName);

    ~World( void )
    {
        Finalize();
    }
};

#endif
