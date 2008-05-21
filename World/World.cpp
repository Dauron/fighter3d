#include <fstream>
#include "World.h"
#include "../Utils/Filesystem.h"

ModelObj * World:: CollideWithRay(xVector3 rayPos, xVector3 rayDir)
{
    assert(m_Valid);

    xVector3 rayEnd = rayPos + rayDir.normalize() * 1000.0f;

    ModelObj *res = NULL;
    xVector3  colPoint;
    float     colDist = 0.f, minDist = 0.f;
    bool      collided = false;

    objectVec::iterator i, j, begin = objects.begin(), end = objects.end();
    for ( i = begin ; i != end ; ++i ) {
        if (cd_RayToMesh.Collide(*i, rayPos, rayEnd, colPoint, colDist)) {
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

    if (deltaTime > 0.05f) deltaTime = 0.05f;
    deltaTime *= g_Speed;

    testsLevel0 = testsLevel1 = testsLevel2 = testsLevel3 = 0;
    time1b = ((time1b*49.0f + time1)/50.0f);
    time2b = ((time2b*49.0f + time2)/50.0f);
    time1 = time2 = 0.f;

    float delta = 0.02f;
    while (deltaTime > EPSILON)
    {
        deltaTime -= delta;
        if (deltaTime < 0.f) { delta += deltaTime; }

        objectVec::iterator i, j, begin = objects.begin(), end = objects.end();
        for ( i = begin ; i != end ; ++i )
            if (! (*i)->phantom)
                for ( j = i + 1; j != end; ++j )
                    if ((*i)->physical || (*j)->physical)
                        if (!(*j)->phantom && cd_MeshToMesh.Collide(*i, *j))
                        {
                            // process collision
                        }

        for ( i = begin ; i != end ; ++i )
            (*i)->PreUpdate();

        for ( i = begin ; i != end ; ++i )
            (*i)->Update(delta);
    }
}

void World:: Initialize()
{
    assert(!m_Valid);

    ModelObj *model;
    SkeletizedObj *modelA;

    lights.clear();
    xLight light;
    light.modified = true;
    light.attenuationConst  = 1.f;
    light.attenuationLinear = 0.004f;
    light.attenuationSquare = 0.0008f;
    light.spotDirection.init(0.f,0.f,-1.f);
    light.spotCutOff = 45.f;
    light.spotAttenuation = 1.f;
    // YELLOW
    light.color.init(0.7f, 0.4f, 0.f, 1.f);
    light.position.init(0.f, 0.f, 10.f);
    light.type = xLight_POINT;
    lights.push_back(light);
    // RED
    light.color.init(0.8f, 0.f, 0.f, 1.f);
    light.position.init(10.f, -5.f, 5.f);
    light.type = xLight_SPOT;
    lights.push_back(light);
    // SKY
    light.color.init(0.4f, 0.4f, 0.4f, 1.f);
    light.position.init(0.f, 0.f, 100.f);
    light.type = xLight_INFINITE;
    light.attenuationLinear = 0.f;
    light.attenuationSquare = 0.f;
    lights.push_back(light);

    if (!g_Test)
    {
        Load("Data/models/dojo/dojo.map");

        model = new ModelObj(-4.0f, -2.0f, 1.0f);
        model->Initialize("Data/models/crate.3dx", "Data/models/crate_fst.3dx", true, false);
        model->mass     = 50.f;
        objects.push_back(model);

        model = new ModelObj(-2.0f, -5.0f, 0.0f);
        model->Initialize("Data/models/crate.3dx", "Data/models/crate_fst.3dx", true, false);
        model->mass     = 50.f;
        objects.push_back(model);

        model = new ModelObj(2.f, 0.f, 0.0f, 0.0f, 0.0f, 45.0f);
        model->Initialize("Data/models/crate.3dx", "Data/models/crate_fst.3dx", true, false);
        model->mass     = 50.f;
        objects.push_back(model);

        model = new ModelObj(10.0f, -2.0f, 0.0f);
        model->Initialize("Data/models/1barbells.3dx");
        objects.push_back(model);

        model = new ModelObj(10.0f, 0.0f, 0.6f);
        model->Initialize("Data/models/2stend.3dx");
        objects.push_back(model);

        shadowCaster = model = new ModelObj(0.0f, -10.0f, 5.0f);
        model->Initialize("Data/models/3vaulting_gym.3dx", "Data/models/3vaulting_gym_fst.3dx", true, false);
        model->mass     = 60.f;
        objects.push_back(model);

        modelA = new SkeletizedObj(-0.3f, -3.2f, -0.21f, 0.0f, 0.0f, 170.0f);
        modelA->Initialize("Data/models/human2.3dx", "Data/models/human2_fst.3dx", false, false);
        modelA->mass     = 70.f;
        modelA->AddAnimation("Data/models/anims/human/yoko-geri2.ska", 4000, 5300);
        modelA->AddAnimation("Data/models/anims/human/garda.ska");
        objects.push_back(modelA);

        modelA = new SkeletizedObj(-0.5f, -1.5f, -0.21f, 0.0f, 0.0f, 0.0f);
        modelA->Initialize("Data/models/human2.3dx", "Data/models/human2_fst.3dx", false, false);
        modelA->mass     = 70.f;
        modelA->AddAnimation("Data/models/anims/human/garda.ska", 0, 4700);
        modelA->AddAnimation("Data/models/anims/human/skulony.ska", 4700);
        modelA->AddAnimation("Data/models/anims/human/kiwa_sie.ska", 4700);
        objects.push_back(modelA);
/*
        modelA = new SkeletizedObj(4.5f, -1.5f, 0.0f, 0.0f, 0.0f, 0.0f);
        modelA->Initialize("Data/models/human2.3dx", "Data/models/human2_fst.3dx");
        modelA->phantom = false;
        modelA->physical = false;
        modelA->mass     = 70.f;
        //modelA->AddAnimation("Data/models/anims/human/idzie.ska");
        //modelA->AddAnimation("Data/models/anims/human/garda.ska");
        objects.push_back(modelA);
*/
        modelA = new SkeletizedObj(5.f, -1.5f, -0.21f, 0.0f, 0.0f, 0.0f);
        modelA->Initialize("Data/models/human3.3dx", "Data/models/human3_fst.3dx", false, false);
        modelA->mass     = 70.f;
        objects.push_back(modelA);

        model = new ModelObj(1.0f, 5.0f, -0.21f);
        model->Initialize("Data/models/wolf.3dx", "Data/models/wolf.3dx", false, false);
        model->mass     = 65.f;
        objects.push_back(model);
    }
    else
    if (g_Test == 1)
    {
        model = new ModelObj(-3.75f, -3.75f, 0.75f, 0.f, 0.f, 0.f);
        model->Initialize("Data/models/crate.3dx", false, false);
        model->transVelocity.x = 5.0f;
        model->mass     = 50.f;
        model->resilience = 0.2f;
        objects.push_back(model);

        model = new ModelObj(-1.0f, -4.0f, 1.0f);
        model->Initialize("Data/models/crate.3dx", false, false);
        model->mass     = 50.f;
        model->resilience = 0.2f;
        objects.push_back(model);

        model = new ModelObj(4.0f, -4.0f, 1.0f);
        model->Initialize("Data/models/crate.3dx", false, false);
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

bool StartsWith(const char *buff, const char *string)
{
    if (!string || !buff) return false;

    for(; *string && *buff; ++string, ++buff)
        if (tolower(*string) != tolower(*buff))
            return false;
    return !*string;
}

void World:: Load(char *mapFileName)
{
    std::ifstream in;

    in.open(mapFileName);
    if (in.is_open())
    {
        std::string dir = Filesystem::GetParentDir(mapFileName);
        char buffer[255];
        ModelObj *model = NULL;

        while (in.good())
        {
            in.getline(buffer, 255);
            if (buffer[0] == '#') continue;

            if (StartsWith(buffer, "model"))
            {
                if (model != NULL)
                    objects.push_back(model);
                model = new ModelObj();

                std::string modelFile = Filesystem::GetFullPath(dir + "/" + (buffer+6));
                model->Initialize(modelFile.c_str());
                continue;
            }
            if (StartsWith(buffer, "position"))
            {
                float x,y,z;
                sscanf(buffer, "position\t%f\t%f\t%f", &x,&y,&z);
                model->Translate(x, y, z);
                continue;
            }
            if (StartsWith(buffer, "rotation"))
            {
                float x,y,z;
                sscanf(buffer, "rotation\t%f\t%f\t%f", &x,&y,&z);
                model->Rotate(x, y, z);
                continue;
            }
            if (StartsWith(buffer, "physical"))
            {
                int b;
                sscanf(buffer, "physical\t%d", &b);
                model->physical = b;
                continue;
            }
            if (StartsWith(buffer, "phantom"))
            {
                int b;
                sscanf(buffer, "phantom\t%d", &b);
                model->phantom = b;
                continue;
            }
            if (StartsWith(buffer, "mass"))
            {
                float mass;
                sscanf(buffer, "mass\t%f", &mass);
                model->mass = mass;
                continue;
            }
            if (StartsWith(buffer, "resilience"))
            {
                float resilience;
                sscanf(buffer, "resilience\t%f", &resilience);
                model->resilience = resilience;
                continue;
            }
        }
        if (model != NULL)
            objects.push_back(model);

        in.close();
    }
}