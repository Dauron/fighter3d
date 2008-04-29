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

    ModelObj *CollideWithRay(xVector3 rayPos, xVector3 rayDir)
    {
        assert(m_Valid);

        xVector3 rayEnd = rayPos + rayDir.normalize() * 1000.0f;

        ModelObj *res = NULL;
        xVector3  colPoint;
        float     colDist = 0, minDist;
        bool      collided = false;

        objectVec::iterator i, j, begin = objects.begin(), end = objects.end();
        for ( i = begin ; i != end ; ++i ) {
            if (rayCollisionDetector.Collide(*i, rayPos, rayEnd, colPoint, colDist)) {
                if (!collided || minDist > colDist) {
                    res = *i;
                    minDist = colDist;
                }
                collided = true;
            }
        }

        return res;
    }

    void Update(float deltaTime)
    {
        assert(m_Valid);

        deltaTime *= g_Speed;

        objectVec::iterator i, j, begin = objects.begin(), end = objects.end();
        for ( i = begin ; i != end ; ++i )
            (*i)->Update(deltaTime);

        for ( i = begin ; i != end ; ++i )
            if (! (*i)->phantom)
                for ( j = i + 1; j != end; ++j )
                    if (!(*j)->phantom && collisionDetector.Collide(*i, *j))
                    {
                        // process collision
                    }
    }

    void Render()
    {
        assert(m_Valid);
        objectVec::iterator i, begin = objects.begin(), end = objects.end();
        for ( i = begin ; i != end ; ++i )
            (*i)->Render();
    }

    ModelObj *Select(int X, int Y)
    {
        std::vector<xDWORD> *objectIDs = ISelectionProvider::Select(X, Y);
        return objectIDs == NULL ? NULL : objects[objectIDs->back()];
    }

    void Initialize()
    {
        assert(!m_Valid);

        ModelObj *model;
        SkeletizedObj *modelA;

        if (!g_Test)
        {
            model = new ModelObj(0.0f, 0.0f, -0.21f);
            model->Initialize("Data/models/arena.3dx");
            model->phantom  = false;
            model->physical = false;
            model->mass     = 100000000.f;
            model->resilience = 0.f;
            objects.push_back(model);

            model = new ModelObj(-4.0f, -2.0f, 0.0f);
            model->Initialize("Data/models/crate.3dx");
            model->phantom = false;
            model->physical = true;
            model->mass     = 50.f;
            model->resilience = 0.2f;
            objects.push_back(model);

            model = new ModelObj(-2.0f, -5.0f, 0.0f);
            model->Initialize("Data/models/crate.3dx");
            model->phantom = false;
            model->physical = true;
            model->mass     = 50.f;
            model->resilience = 0.2f;
            objects.push_back(model);

            model = new ModelObj(2.f, 0.f, 0.0f, 0.0f, 0.0f, 45.0f);
            model->Initialize("Data/models/crate.3dx");
            model->phantom = false;
            model->physical = true;
            model->mass     = 50.f;
            model->resilience = 0.2f;
            objects.push_back(model);

            model = new ModelObj(10.0f, -2.0f, 0.2f);
            model->Initialize("Data/models/1barbells.3dx");
            objects.push_back(model);

            //model = new ModelObj(10.0f, 0.0f, 0.80f);
            //model->Initialize("Data/models/2stend.3dx");
            //objects.push_back(model);

            modelA = new SkeletizedObj(10.0f, 0.0f, 0.80f);
            modelA->Initialize("Data/models/2stend.3dx");
            modelA->AddAnimation("Data/models/anims/2stend_wave.ska");
            objects.push_back(modelA);

            model = new ModelObj(5.0f, -10.0f, 5.7f);
            model->Initialize("Data/models/3vaulting_gym.3dx");
            model->phantom = false;
            model->physical = true;
            model->mass     = 60.f;
            objects.push_back(model);

            modelA = new SkeletizedObj(-0.3f, -3.2f, 0.0f, 0.0f, 0.0f, 170.0f);
            modelA->Initialize("Data/models/human2.3dx");
            modelA->phantom = false;
            modelA->physical = false;
            modelA->mass     = 70.f;
            modelA->AddAnimation("Data/models/anims/human/yoko-geri2.ska", 4000, 5300);
            modelA->AddAnimation("Data/models/anims/human/garda.ska");
            objects.push_back(modelA);

            modelA = new SkeletizedObj(-0.5f, -1.5f, 0.0f, 0.0f, 0.0f, 0.0f);
            modelA->Initialize("Data/models/human2.3dx");
            modelA->phantom = false;
            modelA->physical = false;
            modelA->mass     = 70.f;
            modelA->AddAnimation("Data/models/anims/human/garda.ska", 0, 4700);
            modelA->AddAnimation("Data/models/anims/human/skulony.ska", 4700);
            modelA->AddAnimation("Data/models/anims/human/kiwa_sie.ska", 4700);
            objects.push_back(modelA);

            modelA = new SkeletizedObj(5.f, -1.5f, 0.0f, 0.0f, 0.0f, 0.0f);
            modelA->Initialize("Data/models/human2.3dx");
            modelA->phantom = false;
            modelA->physical = false;
            modelA->mass     = 70.f;
            modelA->AddAnimation("Data/models/anims/human/idzie.ska");
            modelA->AddAnimation("Data/models/anims/human/garda.ska");
            objects.push_back(modelA);

            model = new ModelObj(1.0f, 5.0f, 0.0f);
            model->Initialize("Data/models/wolf.3dx");
            model->phantom = false;
            model->physical = false;
            model->mass     = 65.f;
            objects.push_back(model);
        }
        else
        if (g_Test == 1)
        {
            model = new ModelObj(-3.75f, -3.75f, 0.75f, 0.f, 0.f, 0.f);
            model->Initialize("Data/models/crate.3dx");
            model->phantom = false;
            model->physical = false;
            model->transVelocity.x = 5.0f;
            model->mass     = 50.f;
            model->resilience = 0.2f;
            objects.push_back(model);

            model = new ModelObj(-1.0f, -4.0f, 1.0f);
            model->Initialize("Data/models/crate.3dx");
            model->phantom = false;
            model->physical = false;
            model->mass     = 50.f;
            model->resilience = 0.2f;
            objects.push_back(model);

            model = new ModelObj(4.0f, -4.0f, 1.0f);
            model->Initialize("Data/models/crate.3dx");
            model->phantom = false;
            model->physical = false;
            model->mass     = 50.f;
            model->resilience = 0.2f;
            objects.push_back(model);
        }

        m_Valid = true;
    }

    void Finalize()
    {
        m_Valid = false;
        objectVec::iterator i, begin = objects.begin(), end = objects.end();
        for ( i = begin ; i != end ; ++i )
        {
            (*i)->Finalize();
            delete *i;
    }
        objects.clear();
    }

   ~World( void )
   {
        Finalize();
   }
};

#endif
