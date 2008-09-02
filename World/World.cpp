#include <fstream>
#include "World.h"
#include "SkeletizedObj.h"
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

        if (skyBox) skyBox->FrameUpdate(T_step);
        Interact(T_step, objects);

        if (T_delta > EPSILON)
            FrameEnd();
    }


    Vec_xLight::iterator LT_curr = lights.begin(),
                         LT_last = lights.end();
    for (; LT_curr != LT_last ; ++LT_curr)
        LT_curr->update();
}

void World:: Initialize(std::string MapFileName)
{
    g_CaptureInput.Finalize();
    if (!MapFileName.size())
        MapFileName = "Data/models/level_" + itos( Config::TestCase ) + ".map";
    Load(MapFileName.c_str());
}


void World:: Finalize()
{
    if (skyBox)
    {
        skyBox->Finalize();
        delete skyBox;
        skyBox = NULL;
    }

    Vec_Object::iterator i, begin = objects.begin(), end = objects.end();
    for ( i = begin ; i != end ; ++i )
    {
        (**i).Finalize();
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

        xLight light;
        light.modified = false;
        light.turned_on = true;
        light.attenuationConst  = 1.f;
        light.attenuationLinear = 0.004f;
        light.attenuationSquare = 0.0008f;
        light.spotDirection.init(0.f,0.f,-1.f);
        light.spotCutOff = 45.f;
        light.spotAttenuation = 1.f;

        SkeletizedObj::camera_controled  = NULL;
        SkeletizedObj::network_controled = NULL;
        MX_spawn1.identity();
        MX_spawn2.identity();

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
                if (StartsWith(buffer, "[model]") || StartsWith(buffer, "[skeletized]"))
                {
                    if (model)
                    {
                        if (model->modelFile.size() && model->fastModelFile.size())
                        {
                            model->Initialize(model->modelFile.c_str(), model->fastModelFile.c_str());
                            objects.push_back(model);
                        }
                        else
                        if (model->modelFile.size())
                        {
                            model->Initialize(model->modelFile.c_str());
                            objects.push_back(model);
                        }
                        else
                            delete model;
                    }
                    mode = LoadMode_Model;
                    model = StartsWith(buffer, "[model]") ? new RigidObj() : new SkeletizedObj();
                    model->ApplyDefaults();
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
                    char file[255];
                    sscanf(buffer+6, "%s", file);
                    Load(Filesystem::GetFullPath(dir + "/" + file).c_str());
                    continue;
                }
                if (StartsWith(buffer, "skybox"))
                {
                    char file[255];
                    sscanf(buffer+6, "%s", file);
                    std::string modelFile = Filesystem::GetFullPath(dir + "/" + file);
                    skyBox = new RigidObj();
                    skyBox->Initialize(modelFile.c_str());
                    continue;
                }
                if (StartsWith(buffer, "skycolor"))
                {
                    sscanf(buffer+8, "%f\t%f\t%f", &skyColor.r, &skyColor.g, &skyColor.b);
                    continue;
                }
                if (StartsWith(buffer, "spawn1pos"))
                {
                    float x,y,z;
                    sscanf(buffer+9, "%f\t%f\t%f", &x,&y,&z);
                    MX_spawn1.postTranslateT(xVector3::Create(x,y,z));
                    continue;
                }
                if (StartsWith(buffer, "spawn2pos"))
                {
                    float x,y,z;
                    sscanf(buffer+9, "%f\t%f\t%f", &x,&y,&z);
                    MX_spawn2.postTranslateT(xVector3::Create(x,y,z));
                    continue;
                }
                if (StartsWith(buffer, "spawn1rot"))
                {
                    float x,y,z;
                    sscanf(buffer+9, "%f\t%f\t%f", &x,&y,&z);
                    MX_spawn1 *= xMatrixRotateRad(DegToRad(x), DegToRad(y), DegToRad(z));
                    continue;
                }
                if (StartsWith(buffer, "spawn2rot"))
                {
                    float x,y,z;
                    sscanf(buffer+9, "%f\t%f\t%f", &x,&y,&z);
                    MX_spawn2 *= xMatrixRotateRad(DegToRad(x), DegToRad(y), DegToRad(z));
                    continue;
                }
            }
            if (mode == LoadMode_Model)
            {
                model->LoadLine(buffer, dir);
            }
            if (mode == LoadMode_Light)
            {
                if (StartsWith(buffer, "type"))
                {
                    char type[255];
                    sscanf(buffer+4, "%s", type);
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
                    sscanf(buffer+5, "%d", &b);
                    light.turned_on = b;
                    continue;
                }
                if (StartsWith(buffer, "position"))
                {

                    float x,y,z;
                    sscanf(buffer+8, "%f\t%f\t%f", &x,&y,&z);
                    light.position.init(x, y, z);
                    continue;
                }
                if (StartsWith(buffer, "direction"))
                {

                    float x,y,z;
                    sscanf(buffer+9, "%f\t%f\t%f", &x,&y,&z);
                    light.position.init(-x, -y, -z);
                    continue;
                }
                if (StartsWith(buffer, "color"))
                {

                    float r,g,b;
                    sscanf(buffer+5, "%f\t%f\t%f", &r,&g,&b);
                    light.color.init(r, g, b, 1.f);
                    continue;
                }
                if (StartsWith(buffer, "softness"))
                {
                    sscanf(buffer+8, "%f", &light.softness);
                    continue;
                }
                if (StartsWith(buffer, "spot_dir"))
                {

                    float x,y,z;
                    sscanf(buffer+8, "%f\t%f\t%f", &x,&y,&z);
                    light.spotDirection.init(x, y, z);
                    continue;
                }
                if (StartsWith(buffer, "spot_cut"))
                {
                    sscanf(buffer+8, "%f", &light.spotCutOff);
                    continue;
                }
                if (StartsWith(buffer, "spot_att"))
                {
                    sscanf(buffer+8, "%f", &light.spotAttenuation);
                    continue;
                }
                if (StartsWith(buffer, "att_const"))
                {
                    sscanf(buffer+9, "%f", &light.attenuationConst);
                    continue;
                }
                if (StartsWith(buffer, "att_linear"))
                {
                    sscanf(buffer+10, "%f", &light.attenuationLinear);
                    continue;
                }
                if (StartsWith(buffer, "att_square"))
                {
                    sscanf(buffer+10, "%f", &light.attenuationSquare);
                    continue;
                }
            }
        }
        if (model)
        {
            if (model->modelFile.size() && model->fastModelFile.size())
            {
                model->Initialize(model->modelFile.c_str(), model->fastModelFile.c_str());
                objects.push_back(model);
            }
            else
            if (model->modelFile.size())
            {
                model->Initialize(model->modelFile.c_str());
                objects.push_back(model);
            }
            else
                delete model;
        }
        if (light.modified)
            lights.push_back(light);

        if (SkeletizedObj::camera_controled)
        {
            g_CaptureInput.Finalize();
            bool captureOK = g_CaptureInput.Initialize(SkeletizedObj::camera_controled->ModelGr_Get().xModelP->Spine);
            SkeletizedObj::camera_controled->ControlType = (captureOK)
                ? SkeletizedObj::Control_CaptureInput
                : SkeletizedObj::Control_ComBoardInput;
            SkeletizedObj::camera_controled->FL_auto_movement = true;
        }
        if (SkeletizedObj::network_controled)
        {
            g_NetworkInput.Finalize();
            bool networkOK = g_NetworkInput.Initialize(SkeletizedObj::network_controled->ModelGr_Get().xModelP->Spine);
            SkeletizedObj::network_controled->ControlType = (networkOK)
                ? SkeletizedObj::Control_NetworkInput
                : SkeletizedObj::Control_AI;
            SkeletizedObj::network_controled->FL_auto_movement = true;
        }

        in.close();
    }
}
