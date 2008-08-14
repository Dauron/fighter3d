#include <fstream>
#include "World.h"
#include "../Utils/Filesystem.h"
/*
RigidObj * World:: CollideWithRay(xVector3 rayPos, xVector3 rayDir)
{
    xVector3 rayEnd = rayPos + rayDir.normalize() * 1000.0f;

    RigidObj *res = NULL;
    xVector3  colPoint;
    float     colDist = 0.f, minDist = 0.f;
    bool      collided = false;

    xObjectVector::iterator i, j, begin = objects.begin(), end = objects.end();
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
*/
const xFLOAT TIME_STEP = 0.02f;

void World:: Update(float T_delta)
{
    if (T_delta > 0.05f) T_delta = 0.05f;
    T_delta *= Config::Speed;

    float T_step = TIME_STEP;
    while (T_delta > EPSILON)
    {
        T_delta -= T_step;
        if (T_delta < 0.f) { T_step += T_delta; }

        Interact(T_step, objects);

/*
        xObjectVector::iterator i, j, begin = objects.begin(), end = objects.end();
        for ( i = begin ; i != end ; ++i )
            if (! (*i)->FL_phantom)
                for ( j = i + 1; j != end; ++j )
                    if (!(*i)->FL_stationary || !(*j)->FL_stationary)
                        if (!(*j)->FL_phantom && cd_MeshToMesh.Collide(*i, *j))
                        {
                            // process collision
                        }

        for ( i = begin ; i != end ; ++i )
            (*i)->PreUpdate(delta);

        for ( i = begin ; i != end ; ++i )
            (*i)->Update(delta);
*/
    }
}

void World:: Initialize()
{
    g_CaptureInput.Finalize();
    std::string filename = "Data/models/level_" + itos( Config::TestCase ) + ".map";
    Load(filename.c_str());
}


void World:: Finalize()
{
    if (skyBox)
    {
        skyBox->Finalize();
        delete skyBox;
        skyBox = NULL;
    }

    ObjectVector::iterator i, begin = objects.begin(), end = objects.end();
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
        RigidObj *model = NULL;
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
                    model = new RigidObj();
                    model->ApplyDefaults();
                    fastModelFile.clear();
                    continue;
                }
                if (StartsWith(buffer, "[skeletized]"))
                {
                    mode = LoadMode_Model;
                    if (model != NULL)
                        objects.push_back(model);
                    model = new SkeletizedObj();
                    model->ApplyDefaults();
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
                if (StartsWith(buffer, "skybox"))
                {
                    std::string modelFile = Filesystem::GetFullPath(dir + "/" + (buffer+7));
                    skyBox = new RigidObj();
                    skyBox->Initialize(modelFile.c_str());
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
                    xVector3 *A_iter = model->verletSystem.A_forces;
                    xFLOAT TIME_STEP_INV = 1.f / TIME_STEP;
                    xVector3 speed; speed.init(x*TIME_STEP_INV, y*TIME_STEP_INV, z*TIME_STEP_INV);
                    for (xWORD i = model->verletSystem.I_particles; i; --i, ++A_iter)
                        *A_iter = speed;
                    continue;
                }
                if (StartsWith(buffer, "physical"))
                {
                    int b;
                    sscanf(buffer, "physical\t%d", &b);
                    model->FL_physical = b;
                    continue;
                }
                if (StartsWith(buffer, "locked"))
                {
                    int b;
                    sscanf(buffer, "locked\t%d", &b);
                    model->FL_stationary = b;
                    continue;
                }
                if (StartsWith(buffer, "phantom"))
                {
                    int b;
                    sscanf(buffer, "phantom\t%d", &b);
                    model->FL_phantom = b;
                    continue;
                }
                if (StartsWith(buffer, "mass"))
                {
                    float mass;
                    sscanf(buffer, "mass\t%f", &mass);
                    model->M_mass = mass;
                    continue;
                }
                if (StartsWith(buffer, "resilience"))
                {
                    float resilience;
                    sscanf(buffer, "resilience\t%f", &resilience);
                    model->W_resilience = resilience;
                    continue;
                }
                if (StartsWith(buffer, "shadows"))
                {
                    int b;
                    sscanf(buffer, "shadows\t%d", &b);
                    model->FL_shadowcaster = b;
                    continue;
                }

                if (StartsWith(buffer, "animation"))
                {
                    int start, end = -1;
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
                    g_CaptureInput.Finalize();
                    bool captureOK = g_CaptureInput.Initialize(model->GetModelGr()->Spine);
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
