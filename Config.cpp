#include "Config.h"
#include <cstring>

int   Config::Initialize         = false;
bool  Config::EnableLighting     = true;
bool  Config::EnableFullLighting = true;
bool  Config::EnableShadows      = true;
bool  Config::DisplayShadowVolumes = false;
bool  Config::EnableShaders      = true;
int   Config::PolygonMode        = 0x1B02;

int   Config::TestCase          = 0;
float Config::Speed             = 1.f;
int   Config::ShadowMapSize     = 512;

int   Config::LoggingLevel      = 3;

bool  State::RenderingSelection = false;
bool  State::RenderingShadows   = false;

_Performance Performance;

void _Performance :: Reset()
{
    memset (this, 0, sizeof(_Performance));
    FPSmin = 1000.f;
    _timeCounter = 500.f; // force init snaps at first frame
}

void _Performance :: NextFrame(float ticks)
{
    if (ticks == 0.f)
        ticks = 1.f;

    FPS = 1000.f / ticks;
    _timeCounter += ticks;

    if (_timeCounter > 500.f)
    {
        _timeCounter -= 500.f;
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

    FPSmeanAccum += ticks*FPS;
    FPSmeanCount += ticks;

    if (FPS > FPSmax)
        FPSmax = FPS;
    if (FPS > 0.f && FPS < FPSmin)
        FPSmin = FPS;
}
