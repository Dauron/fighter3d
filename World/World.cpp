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
    deltaTime *= Config::Speed;

    float delta = 0.02f;
    while (deltaTime > EPSILON)
    {
        deltaTime -= delta;
        if (deltaTime < 0.f) { delta += deltaTime; }

        objectVec::iterator i, j, begin = objects.begin(), end = objects.end();
        for ( i = begin ; i != end ; ++i )
            if (! (*i)->phantom)
                for ( j = i + 1; j != end; ++j )
                    if (!(*i)->locked || !(*j)->locked)
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

    g_CaptureInput.Finalize();

    model = new ModelObj();
    model->Initialize("Data/models/night.3dx");
    model->physical = 0;
    model->phantom = 0;
    objects.push_back(model);

    if (!Config::TestCase)
    {
        xLight light;
        light.modified = true;
        light.turned_on = true;
        light.spotDirection.init(0.f,0.f,-1.f);
        light.spotCutOff = 45.f;
        light.spotAttenuation = 1.f;
        // YELLOW
        light.create();
        light.attenuationConst  = 0.9f;
        light.attenuationLinear = 0.005f;
        light.attenuationSquare = 0.0005f;
        light.color.init(1.0f, 0.7f, 0.f, 1.f);
        light.softness = 0.3f;
        light.position.init(0.f, 0.f, 10.f);
        light.type = xLight_POINT;
        lights.push_back(light);
        // RED
        light.create();
        light.attenuationConst  = 1.f;
        light.attenuationLinear = 0.0004f;
        light.attenuationSquare = 0.00008f;
        light.color.init(0.7f, 0.f, 0.f, 1.f);
        light.softness = 0.1f;
        light.position.init(7.f, -5.f, 5.f);
        light.type = xLight_SPOT;
        lights.push_back(light);
        // SKY
        light.create();
        light.color.init(0.05f, 0.05f, 0.05f, 1.f);
        light.softness = 0.8f;
        light.position.init(-20.f, 20.f, 100.f);
        light.type = xLight_INFINITE;
        light.attenuationLinear = 0.f;
        light.attenuationSquare = 0.f;
        lights.push_back(light);

        Load("Data/models/dojo/dojo.map");

        model = new ModelObj(-4.0f, -2.0f, 1.0f);
        model->Initialize("Data/models/crate.3dx", "Data/models/crate_fst.3dx", true, false);
        model->mass     = 50.f;
        model->castsShadows = true;
        objects.push_back(model);

        model = new ModelObj(-2.0f, -5.0f, 0.0f);
        model->Initialize("Data/models/crate.3dx", "Data/models/crate_fst.3dx", true, false);
        model->mass     = 50.f;
        model->castsShadows = true;
        objects.push_back(model);

        model = new ModelObj(2.f, 0.f, 0.0f, 0.0f, 0.0f, 45.0f);
        model->Initialize("Data/models/crate.3dx", "Data/models/crate_fst.3dx", true, false);
        model->mass     = 50.f;
        model->castsShadows = true;
        objects.push_back(model);

        model = new ModelObj(10.0f, -2.0f, -0.2f);
        model->Initialize("Data/models/1barbells.3dx");
        model->castsShadows = true;
        objects.push_back(model);

        model = new ModelObj(10.0f, 0.0f, -0.2f);
        model->Initialize("Data/models/2stend.3dx");
        model->castsShadows = true;
        objects.push_back(model);

        model = new ModelObj(0.0f, -10.0f, 5.0f);
        model->Initialize("Data/models/3vaulting_gym.3dx", "Data/models/3vaulting_gym_fst.3dx", true, false);
        model->mass     = 60.f;
        model->castsShadows = true;
        objects.push_back(model);

        modelA = new SkeletizedObj(-0.3f, -3.2f, -0.21f, 0.0f, 0.0f, 170.0f);
        modelA->Initialize("Data/models/human2.3dx", "Data/models/human2_fst.3dx", false, false);
        modelA->mass     = 70.f;
        modelA->AddAnimation("Data/models/anims/human/yoko-geri2.ska", 4000, 5300);
        modelA->AddAnimation("Data/models/anims/human/garda.ska");
        modelA->castsShadows = true;
        objects.push_back(modelA);

        modelA = new SkeletizedObj(-0.5f, -1.5f, -0.21f, 0.0f, 0.0f, 0.0f);
        modelA->Initialize("Data/models/human2.3dx", "Data/models/human2_fst.3dx", false, false);
        modelA->mass     = 70.f;
        modelA->AddAnimation("Data/models/anims/human/garda.ska", 0, 4700);
        modelA->AddAnimation("Data/models/anims/human/skulony.ska", 4700);
        modelA->AddAnimation("Data/models/anims/human/kiwa_sie.ska", 4700);
        modelA->castsShadows = true;
        objects.push_back(modelA);

        modelA = new SkeletizedObj(4.5f, -1.5f, -0.2f, 0.0f, 0.0f, 0.0f);
        modelA->Initialize("Data/models/human2.3dx", "Data/models/human2_fst.3dx");
        modelA->phantom = false;
        modelA->physical = false;
        modelA->mass     = 70.f;
        bool captureOK = g_CaptureInput.Initialize(modelA->GetModelGr()->spineP);
        modelA->ControlType = (captureOK)
            ? SkeletizedObj::Control_CaptureInput
            : SkeletizedObj::Control_AI;
        objects.push_back(modelA);

        //modelA = new SkeletizedObj(5.f, -1.5f, -0.21f, 0.0f, 0.0f, 0.0f);
        //modelA->Initialize("Data/models/human3.3dx", "Data/models/human3_fst.3dx", false, false);
        //modelA->mass     = 70.f;
        //objects.push_back(modelA);

        //model = new ModelObj(1.0f, 5.0f, -0.21f);
        //model->Initialize("Data/models/wolf.3dx", "Data/models/wolf.3dx", false, false);
        //model->mass     = 65.f;
        //objects.push_back(model);*/
    }
    else
    if (Config::TestCase == 1)
    {
        xLight light;
        light.modified = true;
        light.turned_on = true;
        light.attenuationConst  = 1.f;
        light.attenuationLinear = 0.004f;
        light.attenuationSquare = 0.0008f;
        light.spotDirection.init(0.f,0.f,-1.f);
        light.spotCutOff = 45.f;
        light.spotAttenuation = 1.f;
        // YELLOW
        light.create();
        light.color.init(0.7f, 0.4f, 0.f, 1.f);
        light.position.init(0.5f, 1.f, 1.f);
        light.type = xLight_INFINITE;
        lights.push_back(light);

        model = new ModelObj(-3.75f, -3.75f, 0.75f, 0.f, 0.f, 0.f);
        model->Initialize("Data/models/crate.3dx", "Data/models/crate_fst.3dx", false, false);
        model->transVelocity.x = 5.0f;
        model->locked     = false;
        model->mass       = 50.f;
        model->resilience = 0.2f;
        objects.push_back(model);

        model = new ModelObj(-1.0f, -4.0f, 1.0f);
        model->Initialize("Data/models/crate.3dx", "Data/models/crate_fst.3dx", false, false);
        model->locked     = false;
        model->mass       = 50.f;
        model->resilience = 0.2f;
        objects.push_back(model);

        model = new ModelObj(4.0f, -4.0f, 1.0f);
        model->Initialize("Data/models/crate.3dx", "Data/models/crate_fst.3dx", false, false);
        model->locked     = false;
        model->mass       = 50.f;
        model->resilience = 0.2f;
        objects.push_back(model);
    }
    else
    if (Config::TestCase == 2)
    {
        xLight light;
        light.modified = true;
        light.turned_on = true;
        light.attenuationConst  = 1.f;
        light.attenuationLinear = 0.004f;
        light.attenuationSquare = 0.0008f;
        light.spotDirection.init(0.f,0.f,-1.f);
        light.spotCutOff = 45.f;
        light.spotAttenuation = 1.f;
        // YELLOW
        light.create();
        light.color.init(0.7f, 0.4f, 0.f, 1.f);
        light.position.init(0.5f, 1.f, 1.f);
        light.type = xLight_INFINITE;
        lights.push_back(light);

        modelA = new SkeletizedObj(0.f, 1.f, 1.f, 0.0f, 0.0f, -90.0f);
        modelA->Initialize("Data/models/human2.3dx", "Data/models/human2_fst.3dx");
        modelA->phantom  = false;
        modelA->physical = false;
        modelA->mass     = 70.f;
        bool captureOK = g_CaptureInput.Initialize(modelA->GetModelGr()->spineP);
        modelA->ControlType = (captureOK)
            ? SkeletizedObj::Control_CaptureInput
            : SkeletizedObj::Control_AI;
        objects.push_back(modelA);
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
    lights.clear();
}

bool StartsWith(const char *buff, const char *string)
{
    if (!string || !buff) return false;

    for(; *string && *buff; ++string, ++buff)
        if (tolower(*string) != tolower(*buff))
            return false;
    return !*string;
}

void World:: Load(const char *mapFileName)
{
    std::ifstream in;

    in.open(Filesystem::GetFullPath(mapFileName).c_str());
    if (in.is_open())
    {
        std::string dir = Filesystem::GetParentDir(mapFileName);
        char buffer[255];
        int  len;
        ModelObj *model = NULL;

        while (in.good())
        {
            in.getline(buffer, 255);
            if (buffer[0] == '#') continue;
            len = strlen(buffer);
            if (buffer[len - 1] == '\r') buffer[len - 1] = 0;

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
            if (StartsWith(buffer, "locked"))
            {
                int b;
                sscanf(buffer, "locked\t%d", &b);
                model->locked = b;
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
            if (StartsWith(buffer, "shadows"))
            {
                int b;
                sscanf(buffer, "shadows\t%d", &b);
                model->castsShadows = b;
                continue;
            }
        }
        if (model != NULL)
            objects.push_back(model);

        in.close();
    }
}
