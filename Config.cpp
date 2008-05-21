#include "Config.h"
#include <cstring>

int   Config::Initialize        = false;
bool  Config::EnableLighting    = true;
bool  Config::EnableShaders     = true;
int   Config::PolygonMode       = 0x1B02;

int   Config::TestCase          = 0;
float Config::Speed             = 1.f;
int   Config::ShadowMapSize     = 512;

bool  State::RenderingSelection = false;
bool  State::RenderingShadows   = false;

_Performance Performance;

void _Performance :: Reset()
{
    memset (this, 0, sizeof(_Performance));
    FPSmin = 1000.f;
}

void _Performance :: NextFrame(float ticks)
{
    if (ticks != 0.f)
        FPS = 1.f / (ticks * 0.001f);

    _timeCounter += ticks;
    if (_timeCounter > 500.f)
    {
        _timeCounter -= 500.f;
        snapCollisionDataFillMS      = CollisionDataFillMS_max;
        snapCollisionDeterminationMS = CollisionDeterminationMS_max;
        CollisionDataFillMS_max      = 0.f;
        CollisionDeterminationMS_max = 0.f;
        FPSsnap = FPS;
    }

    CulledElements         = 0;
    CollidedPreTreeLevels  = 0;
    CollidedTreeLevels     = 0;
    CollidedTriangleBounds = 0;
    CollidedTriangles      = 0;

    CollisionDataFillMS_max      = CollisionDataFillMS > CollisionDataFillMS_max ? CollisionDataFillMS : CollisionDataFillMS_max;
    CollisionDeterminationMS_max = CollisionDeterminationMS > CollisionDeterminationMS_max ? CollisionDeterminationMS : CollisionDeterminationMS_max;
    CollisionDataFillMS      = 0.f;
    CollisionDeterminationMS = 0.f;

    FPSmax = FPS > FPSmax ? FPS : FPSmax;
    if (FPS > 0.f)
        FPSmin = FPS < FPSmin ? FPS : FPSmin;
}