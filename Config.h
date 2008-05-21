#ifndef __incl_Config_h
#define __incl_Config_h

struct Config
{
    static int   Initialize;
    static bool  EnableLighting;
    static bool  EnableShaders;
    static int   PolygonMode;

    static int   TestCase;
    static float Speed;
    static int   ShadowMapSize;
};

struct State
{
    static bool RenderingSelection;
    static bool RenderingShadows;
};

struct _Performance
{
    int CulledElements;

    _Performance() : CulledElements(0) {}
};
extern _Performance Performance;

#endif
