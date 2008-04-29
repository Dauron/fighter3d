#ifndef __incl_Scene_h
#define __incl_Scene_h

#ifndef NULL
#define NULL    0
#endif

class Scene
{
public:
    char *sceneName;

    virtual bool Initialize(int left, int top, unsigned int width, unsigned int height)
    {
        Resize (left, top, width, height);
        return true;
    }
    virtual bool Invalidate() { return true; }
    virtual void Terminate() {}
    virtual bool Update(float deltaTime) = 0;
    virtual bool Render() = 0;

    virtual void Resize(int left, int top, unsigned int width, unsigned int height)
    {
        Left        = left;
        Top         = top;
        Width       = width;
        Height      = height;
        AspectRatio = ((float)width)/height;
    }

    Scene() { sceneName = NULL; }
    virtual ~Scene() {}

protected:
    int          Left;
    int          Top;
    unsigned int Width;
    unsigned int Height;
    float        AspectRatio;
};

#endif

