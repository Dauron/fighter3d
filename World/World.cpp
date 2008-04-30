#include "World.h"

ModelObj * World:: CollideWithRay(xVector3 rayPos, xVector3 rayDir)
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

void World:: Update(float deltaTime)
{
    assert(m_Valid);

    if (deltaTime > 2.f) return;

    deltaTime *= g_Speed;

    testsLevel0 = testsLevel1 = testsLevel2 = testsLevel3 = 0;
    time1b = ((time1b*49.0f + time1)/50.0f);
    time2b = ((time2b*49.0f + time2)/50.0f);
    time1 = time2 = 0.f;

    //float delta = 0.001f;
    //while (deltaTime > EPSILON)
    //{
    //    deltaTime -= delta;
    //    if (deltaTime < 0.f) { delta += deltaTime; }

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
    //}
}

void World:: Render()
{
    assert(m_Valid);
    objectVec::iterator i, begin = objects.begin(), end = objects.end();
    for ( i = begin ; i != end ; ++i )
        (*i)->Render();
}

void World:: Initialize()
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

        model = new ModelObj(10.0f, 0.0f, 0.80f);
        model->Initialize("Data/models/2stend.3dx");
        objects.push_back(model);

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
/*
        modelA = new SkeletizedObj(4.5f, -1.5f, 0.0f, 0.0f, 0.0f, 0.0f);
        modelA->Initialize("Data/models/human2.3dx");
        modelA->phantom = false;
        modelA->physical = false;
        modelA->mass     = 70.f;
        //modelA->AddAnimation("Data/models/anims/human/idzie.ska");
        //modelA->AddAnimation("Data/models/anims/human/garda.ska");
        objects.push_back(modelA);
*/
        modelA = new SkeletizedObj(5.f, -1.5f, 0.0f, 0.0f, 0.0f, 0.0f);
        modelA->Initialize("Data/models/human3.3dx");
        modelA->phantom  = false;
        modelA->physical = false;
        modelA->mass     = 70.f;
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


void World:: Finalize()
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