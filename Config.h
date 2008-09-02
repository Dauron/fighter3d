#ifndef __incl_Config_h
#define __incl_Config_h

struct Config
{
    static int   Initialize;
    static bool  EnableLighting;
    static bool  EnableFullLighting;
    static bool  EnableShadows;
    static bool  EnableShaders;
    static int   MultisamplingLevel;
    static int   PolygonMode;
    static int   ShadowMapSize;
    static bool  VSync;
    static int   WindowX;
    static int   WindowY;
    static bool  FullScreen;
    static int   FullScreenX;
    static int   FullScreenY;

    static bool  DisplayShadowVolumes;
    static bool  DisplaySkeleton;
    static bool  DisplayBVH;
    static bool  DisplayCameras;

    static int   TestCase;
    static float Speed;
    static int   LoggingLevel;
    static bool  Save3dsTo3dx;

    static void Load(const char *fileName);
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

    float FPSmeanAccum;
    float FPSmeanCount;

public:
    struct _Shadows
    {
        int shadows;
        int culled;
        int zPass;
        int zFail;
        int zFailS;
        int zFailF;
        int zFailB;

    } Shadows;

    int CulledElements;
    int CulledDiffuseElements;

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

    float T_world;

    void Reset();
    void NextFrame(float ticks);
};
extern _Performance Performance;

#endif
