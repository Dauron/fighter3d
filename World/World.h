#ifndef __incl_World_h
#define __incl_World_h

#include "SkeletizedObj.h"
#include "../OpenGL/ISelectionProvider.h"
#include "../OpenGL/GLShader.h"
#include "../Physics/CollisionDetector.h"
#include "../Physics/RayTrCollisionDetector.h"

class World : public ISelectionProvider
{
    typedef std::vector<ModelObj*> objectVec;
    objectVec objects;

    CollisionDetector collisionDetector;
    RayTrCollisionDetector rayCollisionDetector;

    bool m_Valid;

    virtual void RenderSelect()
    {
        assert(m_Valid);

        int objectID = -1;
        objectVec::iterator i, begin = objects.begin(), end = objects.end();
        for ( i = begin ; i != end ; ++i ) {
            glLoadName(++objectID);
            (*i)->Render();
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
    ModelObj *Select(int X, int Y)
    {
        std::vector<xDWORD> *objectIDs = ISelectionProvider::Select(X, Y);
        return objectIDs == NULL ? NULL : objects[objectIDs->back()];
    }

    void Update(float deltaTime);
    void Render();
    void Initialize();
    void Finalize();

   ~World( void )
   {
        Finalize();
   }
};

#endif
