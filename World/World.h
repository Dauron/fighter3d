#ifndef __incl_World_h
#define __incl_World_h

#include "SkeletizedObj.h"
#include "../OpenGL/ISelectionProvider.h"
#include "../Physics/CD_MeshToMesh.h"
#include "../Physics/CD_RayToMesh.h"

class World : public ISelectionProvider
{
public:
    typedef std::vector<ModelObj*> xObjectVector;

    xObjectVector objects;
    xLightVector  lights;

    ModelObj *skyBox;

private:
    CD_MeshToMesh cd_MeshToMesh;
    CD_RayToMesh  cd_RayToMesh;

    virtual void RenderSelect(const xFieldOfView *FOV)
    {
        int objectID = -1;
        xObjectVector::iterator i, begin = objects.begin(), end = objects.end();
        for ( i = begin ; i != end ; ++i ) {
            glLoadName(++objectID);
            ModelObj &mdl = **i;
            mdl.renderer.RenderVertices(*mdl.GetModelGr(), mdl.modelInstanceGr, xRender::smModel);
        }
    }
    virtual unsigned int CountSelectable()
    {
        return objects.size();
    }

public:

    World( void ) : skyBox(NULL) {}

    ModelObj *CollideWithRay(xVector3 rayPos, xVector3 rayDir);
    ModelObj *Select(const xFieldOfView *FOV, int X, int Y)
    {
        std::vector<xDWORD> *objectIDs = ISelectionProvider::Select(FOV, X, Y);
        return objectIDs == NULL ? NULL : objects[objectIDs->back()];
    }

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
