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

    std::string filename = "Data/models/test_" + itos( Config::TestCase ) + ".map";
    Load(filename.c_str());

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
        std::string fastModelFile;

        xLight light;
        light.modified = false;
        light.turned_on = true;
        light.attenuationConst  = 1.f;
        light.attenuationLinear = 0.004f;
        light.attenuationSquare = 0.0008f;
        light.spotDirection.init(0.f,0.f,-1.f);
        light.spotCutOff = 45.f;
        light.spotAttenuation = 1.f;

        enum LoadMode
        {
            LoadMode_None,
            LoadMode_General,
            LoadMode_Model,
            LoadMode_Light
        } mode = LoadMode_None;

        while (in.good())
        {
            in.getline(buffer, 255);
            if (buffer[0] == 0 || buffer[0] == '#') continue;
            len = strlen(buffer);
            if (buffer[len - 1] == '\r') buffer[len - 1] = 0;

            if (buffer[0] == '[')
            {
                if (StartsWith(buffer, "[general]"))
                {
                    mode = LoadMode_General;
                    continue;
                }
                if (StartsWith(buffer, "[model]"))
                {
                    mode = LoadMode_Model;
                    if (model != NULL)
                        objects.push_back(model);
                    model = new ModelObj();
                    fastModelFile.clear();
                    continue;
                }
                if (StartsWith(buffer, "[skeletized]"))
                {
                    mode = LoadMode_Model;
                    if (model != NULL)
                        objects.push_back(model);
                    model = new SkeletizedObj();
                    fastModelFile.clear();
                    continue;
                }
                if (StartsWith(buffer, "[light]"))
                {
                    mode = LoadMode_Light;
                    if (light.modified)
                        lights.push_back(light);
                    light.create();
                    continue;
                }
                mode = LoadMode_None;
            }
            if (mode == LoadMode_General)
            {
                if (StartsWith(buffer, "import"))
                {
                    Load(Filesystem::GetFullPath(dir + "/" + (buffer+7)).c_str());
                    continue;
                }
            }
            if (mode == LoadMode_Model)
            {
                if (StartsWith(buffer, "fastm"))
                {
                    fastModelFile = Filesystem::GetFullPath(dir + "/" + (buffer+6));
                    continue;
                }
                if (StartsWith(buffer, "model"))
                {
                    std::string modelFile = Filesystem::GetFullPath(dir + "/" + (buffer+6));
                    if (fastModelFile.size())
                        model->Initialize(modelFile.c_str(), fastModelFile.c_str());
                    else
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
                if (StartsWith(buffer, "velocity"))
                {
                    float x,y,z;
                    sscanf(buffer, "velocity\t%f\t%f\t%f", &x,&y,&z);
                    model->transVelocity.init(x, y, z);
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

                if (StartsWith(buffer, "animation"))
                {
                    int start, end;
                    char file[255];
                    sscanf(buffer, "animation\t%s\t%d\t%d", file, &start, &end);
                    std::string animFile = Filesystem::GetFullPath(dir + "/" + file);
                    if (end <= start)
                        ((SkeletizedObj*)model)->AddAnimation(animFile.c_str(), start);
                    else
                        ((SkeletizedObj*)model)->AddAnimation(animFile.c_str(), start, end);
                    continue;
                }

                if (StartsWith(buffer, "camera_controled"))
                {
                    bool captureOK = g_CaptureInput.Initialize(model->GetModelGr()->spine);
                    ((SkeletizedObj*)model)->ControlType = (captureOK)
                        ? SkeletizedObj::Control_CaptureInput
                        : SkeletizedObj::Control_AI;
                    continue;
                }
            }
            if (mode == LoadMode_Light)
            {
                if (StartsWith(buffer, "type"))
                {
                    char *type = buffer+5;
                    if (StartsWith(type, "infinite"))
                        light.type = xLight_INFINITE;
                    else
                    if (StartsWith(type, "point"))
                        light.type = xLight_POINT;
                    else
                    if (StartsWith(type, "spot"))
                        light.type = xLight_SPOT;
                    continue;
                }
                if (StartsWith(buffer, "state"))
                {
                    int b;
                    sscanf(buffer, "state\t%d", &b);
                    light.turned_on = b;
                    continue;
                }
                if (StartsWith(buffer, "position"))
                {

                    float x,y,z;
                    sscanf(buffer, "position\t%f\t%f\t%f", &x,&y,&z);
                    light.position.init(x, y, z);
                    continue;
                }
                if (StartsWith(buffer, "direction"))
                {

                    float x,y,z;
                    sscanf(buffer, "direction\t%f\t%f\t%f", &x,&y,&z);
                    light.position.init(-x, -y, -z);
                    continue;
                }
                if (StartsWith(buffer, "color"))
                {

                    float r,g,b;
                    sscanf(buffer, "color\t%f\t%f\t%f", &r,&g,&b);
                    light.color.init(r, g, b, 1.f);
                    continue;
                }
                if (StartsWith(buffer, "softness"))
                {
                    sscanf(buffer, "softness\t%f", &light.softness);
                    continue;
                }
                if (StartsWith(buffer, "spot_dir"))
                {

                    float x,y,z;
                    sscanf(buffer, "spot_dir\t%f\t%f\t%f", &x,&y,&z);
                    light.spotDirection.init(x, y, z);
                    continue;
                }
                if (StartsWith(buffer, "spot_cut"))
                {
                    sscanf(buffer, "spot_cut\t%f", &light.spotCutOff);
                    continue;
                }
                if (StartsWith(buffer, "spot_att"))
                {
                    sscanf(buffer, "spot_att\t%f", &light.spotAttenuation);
                    continue;
                }
                if (StartsWith(buffer, "att_const"))
                {
                    sscanf(buffer, "att_const\t%f", &light.attenuationConst);
                    continue;
                }
                if (StartsWith(buffer, "att_linear"))
                {
                    sscanf(buffer, "att_linear\t%f", &light.attenuationLinear);
                    continue;
                }
                if (StartsWith(buffer, "att_square"))
                {
                    sscanf(buffer, "att_square\t%f", &light.attenuationSquare);
                    continue;
                }
            }
        }
        if (model != NULL)
            objects.push_back(model);
        if (light.modified)
            lights.push_back(light);

        in.close();
    }
}
