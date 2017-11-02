#include <fstream>
#include "Utils/Filesystem.h"
#include "Utils/Utils.h"
#include "Utils/Stat.h"
#include <cstring>

#include "Config.h"

int   Config::Initialize           = false;
bool  Config::EnableLighting       = true;
bool  Config::EnableShaders        = true;
int   Config::MultisamplingLevel   = 0;
bool  Config::VSync                = false;
int   Config::WindowX              = 800;
int   Config::WindowY              = 600;
bool  Config::FullScreen           = false;
int   Config::FullScreenX          = 800;
int   Config::FullScreenY          = 600;

bool  Config::EnableConsole      = false;
char* Config::Scene              = strdup("menu");

int   Config::LoggingLevel       = 3;

bool  State::RenderingSelection  = false;

_Performance Performance;

void _Performance :: RegisterStatPage()
{
    StatPage *page = new StatPage();
    page->Name = "Performance";

    Stat_IntPtr *stat = new Stat_IntPtr();
    stat->Create("RenderedBirds", RenderedBirds);
    page->Add(*stat);

    g_StatMgr.Add(*page);
}

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
        if (FPSmeanCount > 0.f)
            FPSsnap = FPSmeanAccum / FPSmeanCount;
        else
            FPSsnap = FPS;
        FPSmeanAccum = 0.f;
        FPSmeanCount = 0;
        FPSmin = 1000.f;
        FPSmax = 0.f;
    }

    RenderedBirds = 0;

    FPSmeanAccum += T_delta*FPS;
    FPSmeanCount += T_delta;

    if (FPS > FPSmax)
        FPSmax = FPS;
    if (FPS > 0.f && FPS < FPSmin)
        FPSmin = FPS;
}

#include "World/Map.h"

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
            LoadMode_General,
            LoadMode_Map,
            LoadMode_Quarter,
            LoadMode_Bird,
            LoadMode_Hawk
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
                if (StartsWith(buffer, "[general]"))
                {
                    mode = LoadMode_General;
                    continue;
                }
                if (StartsWith(buffer, "[map]"))
                {
                    mode = LoadMode_Map;
                    continue;
                }
                if (StartsWith(buffer, "[quarter]"))
                {
                    mode = LoadMode_Quarter;
                    continue;
                }
                if (StartsWith(buffer, "[bird]"))
                {
                    mode = LoadMode_Bird;
                    continue;
                }
                if (StartsWith(buffer, "[hawk]"))
                {
                    mode = LoadMode_Hawk;
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
                    Config::EnableLighting = level;
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
            }
            if (mode == LoadMode_General)
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
                if (StartsWith(buffer, "logging"))
                {
                    int level;
                    sscanf(buffer+7, "%d", &level);
                    Config::LoggingLevel = level;
                    continue;
                }
            }
            if (mode == LoadMode_Map)
            {
                World::Map::LoadConfigLine(buffer);
                continue;
            }
            if (mode == LoadMode_Quarter)
            {
                World::Quarter::LoadConfigLine(buffer);
                continue;
            }
            if (mode == LoadMode_Bird)
            {
                World::Bird::LoadConfigLine(buffer);
                continue;
            }
            if (mode == LoadMode_Hawk)
            {
                World::Hawk::LoadConfigLine(buffer);
                continue;
            }
        }

        in.close();
    }
}
