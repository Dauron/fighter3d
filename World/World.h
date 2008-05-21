#ifndef __incl_World_h
#define __incl_World_h

#include "SkeletizedObj.h"
#include "../OpenGL/ISelectionProvider.h"
#include "../Physics/CD_MeshToMesh.h"
#include "../Physics/CD_RayToMesh.h"

class World : public ISelectionProvider
{
public:
    typedef std::vector<ModelObj*> objectVec;
    typedef std::vector<xLight>    lightsVec;
    objectVec objects;
    lightsVec lights;

private:
    CD_MeshToMesh cd_MeshToMesh;
    CD_RayToMesh  cd_RayToMesh;

    bool m_Valid;

    virtual void RenderSelect(const xFieldOfView *FOV)
    {
        assert(m_Valid);

        int objectID = -1;
        objectVec::iterator i, begin = objects.begin(), end = objects.end();
        for ( i = begin ; i != end ; ++i ) {
            glLoadName(++objectID);
            ModelObj &mdl = **i;
            mdl.renderer.RenderVertices(*mdl.GetModelGr(), mdl.modelInstanceGr, xRender::smModel);
            //(*i)->Render(false, *FOV);
            //(*i)->Render(true,  *FOV);
        }
    }
    virtual unsigned int CountSelectable()
    {
        return objects.size();
    }

public:

    bool IsValid() { return m_Valid; }

    World( void ) : m_Valid(false) {}

    ModelObj *CollideWithRay(xVector3 rayPos, xVector3 rayDir);
    ModelObj *Select(const xFieldOfView *FOV, int X, int Y)
    {
        std::vector<xDWORD> *objectIDs = ISelectionProvider::Select(FOV, X, Y);
        return objectIDs == NULL ? NULL : objects[objectIDs->back()];
    }

    void Load       (const char *mapFileName);
    void Initialize ();
    void Update     (float deltaTime);
    void Finalize   ();


    ~World( void )
    {
        Finalize();
    }
};

#endif
