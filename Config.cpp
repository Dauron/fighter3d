#include "Config.h"

int   Config::Initialize        = false;
bool  Config::EnableLighting    = true;
bool  Config::EnableShaders     = false;
int   Config::PolygonMode       = 0x1B02;

int   Config::TestCase          = 0;
float Config::Speed             = 1.f;
int   Config::ShadowMapSize     = 512;

bool  State::RenderingSelection = false;
bool  State::RenderingShadows   = false;

_Performance Performance;
//int   Performance::CulledElements = 0;