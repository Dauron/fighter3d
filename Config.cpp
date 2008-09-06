#include <fstream>
#include "Utils/Filesystem.h"
#include "Utils/Utils.h"
#include <cstring>

#include "Config.h"

int   Config::Initialize           = false;
bool  Config::EnableLighting       = true;
bool  Config::EnableFullLighting   = true;
bool  Config::EnableShadows        = true;
bool  Config::EnableShaders        = true;
int   Config::MultisamplingLevel   = 0;
int   Config::PolygonMode          = 0x1B02;
int   Config::ShadowMapSize        = 512;
bool  Config::VSync                = false;
int   Config::WindowX              = 800;
int   Config::WindowY              = 600;
bool  Config::FullScreen           = false;
int   Config::FullScreenX          = 800;
int   Config::FullScreenY          = 600;


bool  Config::DisplayShadowVolumes = false;
bool  Config::DisplaySkeleton      = false;
bool  Config::DisplayBVH           = false;
bool  Config::DisplayCameras       = false;

bool  Config::EnableConsole      = false;
char* Config::Scene              = strdup("menu");
int   Config::TestCase           = 0;
float Config::Speed              = 1.f;

int   Config::LoggingLevel       = 3;
bool  Config::Save3dsTo3dx       = false;

bool  State::RenderingSelection  = false;
bool  State::RenderingShadows    = false;

_Performance Performance;

void _Performance :: Clear()
{
    memset (this, 0, sizeof(_Performance));
    FPSmin = 1000.f;
    _timeCounter = 0.5f; // force init snaps at first frame
}

void _Performance :: Update(float T_delta)
{
    if (T_delta == 0.f)
        T_delta = 0.001f;

    FPS = 1.f / T_delta;
    _timeCounter += T_delta;

    if (_timeCounter > 0.5f)
    {
        _timeCounter -= 0.5f;
        snapCollisionDataFillMS      = CollisionDataFillMS_max;
        snapCollisionDeterminationMS = CollisionDeterminationMS_max;
        CollisionDataFillMS_max      = 0.f;
        CollisionDeterminationMS_max = 0.f;
        if (FPSmeanCount > 0.f)
            FPSsnap = FPSmeanAccum / FPSmeanCount;
        else
            FPSsnap = FPS;
        FPSmeanAccum = 0.f;
        FPSmeanCount = 0;
        FPSmin = 1000.f;
        FPSmax = 0.f;
    }

    CulledElements         = 0;
    CulledDiffuseElements  = 0;
    CollidedPreTreeLevels  = 0;
    CollidedTreeLevels     = 0;
    CollidedTriangleBounds = 0;
    CollidedTriangles      = 0;

    memset(&Shadows, 0, sizeof(_Shadows));

    CollisionDataFillMS_max      = CollisionDataFillMS > CollisionDataFillMS_max ? CollisionDataFillMS : CollisionDataFillMS_max;
    CollisionDeterminationMS_max = CollisionDeterminationMS > CollisionDeterminationMS_max ? CollisionDeterminationMS : CollisionDeterminationMS_max;
    CollisionDataFillMS      = 0.f;
    CollisionDeterminationMS = 0.f;

    FPSmeanAccum += T_delta*FPS;
    FPSmeanCount += T_delta;

    if (FPS > FPSmax)
        FPSmax = FPS;
    if (FPS > 0.f && FPS < FPSmin)
        FPSmin = FPS;
}

void Config :: Load(const char *fileName)
{
    std::ifstream in;
    in.open(Filesystem::GetFullPath(fileName).c_str());
    if (in.is_open())
    {
        std::string dir = Filesystem::GetParentDir(fileName);
        char buffer[255];
        int  len;

        enum LoadMode
        {
            LoadMode_None,
            LoadMode_Graphics,
            LoadMode_Gameplay
        } mode = LoadMode_None;

        while (in.good())
        {
            in.getline(buffer, 255);
            if (buffer[0] == 0 || buffer[0] == '#') continue;
            len = strlen(buffer);
            if (buffer[len - 1] == '\r') buffer[len - 1] = 0;

            if (*buffer == '[')
            {
                if (StartsWith(buffer, "[graphics]"))
                {
                    mode = LoadMode_Graphics;
                    continue;
                }
                if (StartsWith(buffer, "[gameplay]"))
                {
                    mode = LoadMode_Gameplay;
                    continue;
                }
                mode = LoadMode_None;
                continue;
            }
            if (mode == LoadMode_Graphics)
            {
                if (StartsWith(buffer, "lighting"))
                {
                    int level;
                    sscanf(buffer+8, "%d", &level);
                    Config::EnableLighting     = level > 0;
                    Config::EnableFullLighting = level > 1;
                    continue;
                }
                if (StartsWith(buffer, "shadows"))
                {
                    int level;
                    sscanf(buffer+7, "%d", &level);
                    Config::EnableShadows = level;
                    continue;
                }
                if (StartsWith(buffer, "shaders"))
                {
                    int level;
                    sscanf(buffer+7, "%d", &level);
                    Config::EnableShaders = level;
                    continue;
                }
                if (StartsWith(buffer, "multisampling"))
                {
                    int level;
                    sscanf(buffer+13, "%d", &level);
                    Config::MultisamplingLevel = level;
                    continue;
                }
                if (StartsWith(buffer, "shadowmap"))
                {
                    int level;
                    sscanf(buffer+9, "%d", &level);
                    Config::ShadowMapSize = level;
                    continue;
                }
                if (StartsWith(buffer, "vsync"))
                {
                    int level;
                    sscanf(buffer+5, "%d", &level);
                    Config::VSync = level;
                    continue;
                }
                if (StartsWith(buffer, "windowx"))
                {
                    int level;
                    sscanf(buffer+7, "%d", &level);
                    Config::WindowX = level;
                    continue;
                }
                if (StartsWith(buffer, "windowy"))
                {
                    int level;
                    sscanf(buffer+7, "%d", &level);
                    Config::WindowY = level;
                    continue;
                }
                if (StartsWith(buffer, "fullscreenx"))
                {
                    int level;
                    sscanf(buffer+11, "%d", &level);
                    Config::FullScreenX = level;
                    continue;
                }
                if (StartsWith(buffer, "fullscreeny"))
                {
                    int level;
                    sscanf(buffer+11, "%d", &level);
                    Config::FullScreenY = level;
                    continue;
                }
                if (StartsWith(buffer, "fullscreen"))
                {
                    int level;
                    sscanf(buffer+10, "%d", &level);
                    Config::FullScreen = level;
                    continue;
                }
                if (StartsWith(buffer, "show_shadowvolumes"))
                {
                    int level;
                    sscanf(buffer+18, "%d", &level);
                    Config::DisplayShadowVolumes = level;
                    continue;
                }
                if (StartsWith(buffer, "show_skeleton"))
                {
                    int level;
                    sscanf(buffer+13, "%d", &level);
                    Config::DisplaySkeleton = level;
                    continue;
                }
                if (StartsWith(buffer, "show_bvh"))
                {
                    int level;
                    sscanf(buffer+8, "%d", &level);
                    Config::DisplayBVH = level;
                    continue;
                }
                if (StartsWith(buffer, "show_cameras"))
                {
                    int level;
                    sscanf(buffer+12, "%d", &level);
                    Config::DisplayCameras = level;
                    continue;
                }
            }
            if (mode == LoadMode_Gameplay)
            {
                if (StartsWith(buffer, "console"))
                {
                    int level;
                    sscanf(buffer+7, "%d", &level);
                    Config::EnableConsole = level;
                    continue;
                }
                if (StartsWith(buffer, "scene"))
                {
                    Config::Scene = strdup( ReadSubstring(buffer+5) );
                    continue;
                }
                if (StartsWith(buffer, "level"))
                {
                    int level;
                    sscanf(buffer+5, "%d", &level);
                    Config::TestCase = level;
                    continue;
                }
                if (StartsWith(buffer, "speed"))
                {
                    float level;
                    sscanf(buffer+5, "%f", &level);
                    Config::Speed = level;
                    continue;
                }
                if (StartsWith(buffer, "logging"))
                {
                    int level;
                    sscanf(buffer+7, "%d", &level);
                    Config::LoggingLevel = level;
                    continue;
                }
                if (StartsWith(buffer, "3dsTo3dx"))
                {
                    int level;
                    sscanf(buffer+8, "%d", &level);
                    Config::Save3dsTo3dx = level;
                    continue;
                }
            }
        }

        in.close();
    }
}
