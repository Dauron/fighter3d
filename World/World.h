#ifndef __incl_World_h
#define __incl_World_h

#include "SkeletizedObj.h"
#include "../Physics/CD_MeshToMesh.h"
#include "../Physics/CD_RayToMesh.h"

class World
{
public:
    typedef std::vector<ModelObj*> xObjectVector;

    xObjectVector objects;
    xLightVector  lights;

    ModelObj *skyBox;

private:
    CD_MeshToMesh cd_MeshToMesh;
    CD_RayToMesh  cd_RayToMesh;

public:

    World( void ) : skyBox(NULL) {}

    ModelObj *CollideWithRay(xVector3 rayPos, xVector3 rayDir);

    void Initialize ();
    void Update     (float deltaTime);
    void Finalize   ();

    void Invalidate()
    {
        xLightVector::iterator light, begin = lights.begin(), end = lights.end();
        for (light=begin; light!=end; ++light)
            light->modified = true;

        xObjectVector::iterator model, beginM = objects.begin(), endM = objects.end();
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
