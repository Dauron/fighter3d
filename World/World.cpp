#include <fstream>
#include "World.h"
#include "../Utils/Filesystem.h"
#include "../MotionCapture/CaptureInput.h"
#include "../Multiplayer/NetworkInput.h"

const xFLOAT TIME_STEP = 0.02f;

void World:: FrameUpdate(float T_delta)
{
    if (T_delta > 0.05f) T_delta = 0.05f;
    T_delta *= Config::Speed;

    bool FL_firstStep = true;

    float T_step = TIME_STEP;
    while (T_delta > EPSILON)
    {
        T_delta -= T_step;
        if (T_delta < 0.f) { T_step += T_delta; }

        Performance.T_world += T_step;

        if (FL_firstStep)
            FL_firstStep = false;
        else
            FrameStart();

        Interact(T_step, objects);

        if (T_delta > EPSILON)
            FrameEnd();
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
        std::string fastModelFile, modelFile;

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
                    if (model)
                    {
                        if (modelFile.size() && fastModelFile.size())
                        {
                            model->Initialize(modelFile.c_str(), fastModelFile.c_str());
                            objects.push_back(model);
                        }
                        else
                        if (modelFile.size())
                        {
                            model->Initialize(modelFile.c_str());
                            objects.push_back(model);
                        }
                        else
                            delete model;
                    }
                    mode = LoadMode_Model;
                    model = new RigidObj();
                    model->ApplyDefaults();
                    fastModelFile.clear();
                    modelFile.clear();
                    continue;
                }
                if (StartsWith(buffer, "[skeletized]"))
                {
                    if (model)
                    {
                        if (modelFile.size() && fastModelFile.size())
                        {
                            model->Initialize(modelFile.c_str(), fastModelFile.c_str());
                            objects.push_back(model);
                        }
                        else
                        if (modelFile.size())
                        {
                            model->Initialize(modelFile.c_str());
                            objects.push_back(model);
                        }
                        else
                            delete model;
                    }
                    mode = LoadMode_Model;
                    model = new SkeletizedObj();
                    model->ApplyDefaults();
                    fastModelFile.clear();
                    modelFile.clear();
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
                    modelFile = Filesystem::GetFullPath(dir + "/" + (buffer+6));
                    continue;
                }
                if (StartsWith(buffer, "customBVH"))
                {
                    int customBVH;
                    sscanf(buffer, "customBVH\t%f", &customBVH);
                    model->FL_customBVH = customBVH;
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
                    model->ApplyAcceleration(xVector3::Create(x,y,z), 1.f);
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
                if (StartsWith(buffer, "restitution"))
                {
                    float restitution;
                    sscanf(buffer, "restitution\t%f", &restitution);
                    model->W_restitution = restitution;
                    continue;
                }
                if (StartsWith(buffer, "restitution_self"))
                {
                    float restitution;
                    sscanf(buffer, "restitution_self\t%f", &restitution);
                    model->W_restitution_self = restitution;
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
                        ((SkeletizedObj*)model)->actions.AddAnimation(animFile.c_str(), start);
                    else
                        ((SkeletizedObj*)model)->actions.AddAnimation(animFile.c_str(), start, end);
                    continue;
                }

                if (StartsWith(buffer, "control"))
                {
                    char name[255];
                    sscanf(buffer, "control\t%s", name);

                    if (StartsWith(name, "camera"))
                    {
                        g_CaptureInput.Finalize();
                        bool captureOK = g_CaptureInput.Initialize(model->GetModelGr()->Spine);
                        ((SkeletizedObj*)model)->ControlType = (captureOK)
                            ? SkeletizedObj::Control_CaptureInput
                            : SkeletizedObj::Control_ComBoardInput;
                    }
                    else
                    if (StartsWith(name, "network"))
                    {
                        g_NetworkInput.Finalize();
                        bool networkOK = g_NetworkInput.Initialize(model->GetModelGr()->Spine);
                        ((SkeletizedObj*)model)->ControlType = (networkOK)
                            ? SkeletizedObj::Control_NetworkInput
                            : SkeletizedObj::Control_AI;
                    }
                    else
                    if (StartsWith(name, "comboard"))
                        ((SkeletizedObj*)model)->ControlType = SkeletizedObj::Control_ComBoardInput;
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
        if (model)
        {
            if (modelFile.size() && fastModelFile.size())
            {
                model->Initialize(modelFile.c_str(), fastModelFile.c_str());
                objects.push_back(model);
            }
            else
            if (modelFile.size())
            {
                model->Initialize(modelFile.c_str());
                objects.push_back(model);
            }
            else
                delete model;
        }
        if (light.modified)
            lights.push_back(light);

        in.close();
    }
}
