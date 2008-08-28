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
        RigidObj      *model = NULL;
        SkeletizedObj *camera_controled  = NULL;
        SkeletizedObj *network_controled = NULL;
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
            }
            if (mode == LoadMode_Model)
            {
                if (StartsWith(buffer, "fastm"))
                {
                    char file[255];
                    sscanf(buffer+5, "%s", file);
                    fastModelFile = Filesystem::GetFullPath(dir + "/" + file);
                    continue;
                }
                if (StartsWith(buffer, "model"))
                {
                    char file[255];
                    sscanf(buffer+5, "%s", file);
                    modelFile = Filesystem::GetFullPath(dir + "/" + file);
                    continue;
                }
                if (StartsWith(buffer, "customBVH"))
                {
                    int customBVH;
                    sscanf(buffer+9, "%d", &customBVH);
                    model->FL_customBVH = customBVH;
                    continue;
                }
                if (StartsWith(buffer, "position"))
                {
                    float x,y,z;
                    sscanf(buffer+8, "%f\t%f\t%f", &x,&y,&z);
                    model->Translate(x, y, z);
                    continue;
                }
                if (StartsWith(buffer, "rotation"))
                {
                    float x,y,z;
                    sscanf(buffer+8, "%f\t%f\t%f", &x,&y,&z);
                    model->Rotate(x, y, z);
                    continue;
                }
                if (StartsWith(buffer, "velocity"))
                {
                    float x,y,z;
                    sscanf(buffer+8, "%f\t%f\t%f", &x,&y,&z);
                    model->ApplyAcceleration(xVector3::Create(x,y,z), 1.f);
                    continue;
                }
                if (StartsWith(buffer, "physical"))
                {
                    int b;
                    sscanf(buffer+8, "%d", &b);
                    model->FL_physical = b;
                    continue;
                }
                if (StartsWith(buffer, "locked"))
                {
                    int b;
                    sscanf(buffer+6, "%d", &b);
                    model->FL_stationary = b;
                    continue;
                }
                if (StartsWith(buffer, "phantom"))
                {
                    int b;
                    sscanf(buffer+7, "%d", &b);
                    model->FL_phantom = b;
                    continue;
                }
                if (StartsWith(buffer, "mass"))
                {
                    float mass;
                    sscanf(buffer+4, "%f", &mass);
                    model->M_mass = mass;
                    continue;
                }
                if (StartsWith(buffer, "restitution"))
                {
                    float restitution;
                    sscanf(buffer+11, "%f", &restitution);
                    model->W_restitution = restitution;
                    continue;
                }
                if (StartsWith(buffer, "restitution_self"))
                {
                    float restitution;
                    sscanf(buffer+16, "%f", &restitution);
                    model->W_restitution_self = restitution;
                    continue;
                }
                if (StartsWith(buffer, "shadows"))
                {
                    int b;
                    sscanf(buffer+7, "%d", &b);
                    model->FL_shadowcaster = b;
                    continue;
                }

                if (StartsWith(buffer, "animation"))
                {
                    int start = 0, end = -1;
                    char file[255];
                    sscanf(buffer+9, "%s\t%d\t%d", file, &start, &end);
                    std::string animFile = Filesystem::GetFullPath(dir + "/" + file);
                    if (end <= start)
                        ((SkeletizedObj*)model)->actions.AddAnimation(animFile.c_str(), start);
                    else
                        ((SkeletizedObj*)model)->actions.AddAnimation(animFile.c_str(), start, end);
                    continue;
                }

                if (StartsWith(buffer, "style"))
                {
                    char file[255];
                    sscanf(buffer+5, "%s", file);
                    ((SkeletizedObj*)model)->comBoard.FileName = Filesystem::GetFullPath(dir + "/" + file);
                    continue;
                }
                if (StartsWith(buffer, "control"))
                {
                    char name[255];
                    sscanf(buffer+7, "%s", name);

                    if (StartsWith(name, "camera"))
                        camera_controled = (SkeletizedObj*)model;
                    else
                    if (StartsWith(name, "network"))
                        network_controled = (SkeletizedObj*)model;
                    else
                    if (StartsWith(name, "comboard"))
                    {
                        ((SkeletizedObj*)model)->ControlType = SkeletizedObj::Control_ComBoardInput;
                        ((SkeletizedObj*)model)->FL_auto_movement = false;
                    }
                    continue;
                }
                if (StartsWith(buffer, "enemy"))
                {
                    int b;
                    sscanf(buffer+5, "%d", &b);
                    SkeletizedObj &so    = *(SkeletizedObj*)model;
                    so.Tracker.Mode      = Math::Tracking::ObjectTracker::TRACK_OBJECT;
                    so.Tracker.ID_object = b;
                }
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

        if (camera_controled)
        {
            g_CaptureInput.Finalize();
            bool captureOK = g_CaptureInput.Initialize(camera_controled->ModelGr_Get().xModelP->Spine);
            camera_controled->ControlType = (captureOK)
                ? SkeletizedObj::Control_CaptureInput
                : SkeletizedObj::Control_ComBoardInput;
            camera_controled->FL_auto_movement = true;
        }
        if (network_controled)
        {
            g_NetworkInput.Finalize();
            bool networkOK = g_NetworkInput.Initialize(model->ModelGr_Get().xModelP->Spine);
            network_controled->ControlType = (networkOK)
                ? SkeletizedObj::Control_NetworkInput
                : SkeletizedObj::Control_AI;
            camera_controled->FL_auto_movement = true;
        }

        in.close();
    }
}
