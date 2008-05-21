#ifndef __incl_Config_h
#define __incl_Config_h

struct Config
{
    static int   Initialize;
    static bool  EnableLighting;
    static bool  EnableFullLighting;
    static bool  EnableShadows;
    static bool  DisplayShadowVolumes;
    static bool  EnableShaders;
    static int   PolygonMode;

    static int   TestCase;
    static float Speed;
    static int   ShadowMapSize;
    static int   LoggingLevel;
};

struct State
{
    static bool RenderingSelection;
    static bool RenderingShadows;
};

struct _Performance
{
private:
    float _timeCounter;

    float CollisionDataFillMS_max;
    float CollisionDeterminationMS_max;

public:
    int CulledElements;

    int CollidedPreTreeLevels;
    int CollidedTreeLevels;
    int CollidedTriangleBounds;
    int CollidedTriangles;

    float CollisionDataFillMS;
    float CollisionDeterminationMS;
    float snapCollisionDataFillMS;
    float snapCollisionDeterminationMS;

    float FPS;
    float FPSmin;
    float FPSmax;
    float FPSsnap;

    void Reset();
    void NextFrame(float ticks);
};
extern _Performance Performance;

#endif
