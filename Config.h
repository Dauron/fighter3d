#ifndef __incl_Config_h
#define __incl_Config_h

struct Config
{
    static int   Initialize;
    static bool  EnableLighting;
    static bool  EnableShaders;
    static int   MultisamplingLevel;
    static bool  VSync;
    static int   WindowX;
    static int   WindowY;
    static bool  FullScreen;
    static int   FullScreenX;
    static int   FullScreenY;

    static bool  EnableConsole;
    static char* Scene;
    static int   LoggingLevel;

    static void Load(const char *fileName);

    ~Config()
    { if (Scene) delete[] Scene; }
};

struct State
{
    static bool RenderingSelection;
};

struct _Performance
{
private:
    float _timeCounter;

    float FPSmeanAccum;
    float FPSmeanCount;

public:
    int RenderedBirds;

    float FPS;
    float FPSmin;
    float FPSmax;
    float FPSsnap;

    float T_world;

    void Clear();
    void Update(float T_delta);
    void RegisterStatPage();
};
extern _Performance Performance;

#endif
