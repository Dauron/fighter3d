#ifndef __incl_Text3d_h
#define __incl_Text3d_h

#include "../BaseObj.h"
#include "../../OpenGL/Fonts/FontMgr.h"
#include <string>

class Text3d : public BaseObj
{
public:
    Text3d (std::string text, HFont font) : BaseObj(), Text(text), Font(font)
    { Initialize(); }
    Text3d (std::string text, HFont font, GLfloat x, GLfloat y, GLfloat z)
      : BaseObj(x,z,y), Text(text), Font(font)
    { Initialize(); }
    Text3d (std::string text, HFont font, GLfloat x, GLfloat y, GLfloat z,
         GLfloat rotX, GLfloat rotY, GLfloat rotZ)
      : BaseObj(x,z,y, rotX,rotZ,rotY), Text(text), Font(font)
    { Initialize(); }

    virtual void SetPosition(GLfloat x, GLfloat y, GLfloat z)
    {
        BaseObj::SetPosition(x,z,y);
    }
    virtual GLfloat *GetPosition()
    {
        GLfloat *res = BaseObj::GetPosition();
        GLfloat tmp = res[3];
        res[2] = res[3];
        res[3] = tmp;
        return res;
    }

    virtual void SetRotation(GLfloat rotX, GLfloat rotY, GLfloat rotZ)
    {
        BaseObj::SetRotation(rotX,rotZ,rotY);
    }
    virtual GLfloat *GetRotation()
    {
        GLfloat *res = BaseObj::GetRotation();
        GLfloat tmp = res[3];
        res[2] = res[3];
        res[3] = tmp;
        return res;
    }

    std::string Text;
    GLubyte     Color[4];

protected:
    HFont Font;

    void RenderObject()
    {
        g_TextureMgr.DisableTextures();
        glColor4ubv(Color);

        if (g_FontMgr.IsHandleValid(Font))
            g_FontMgr.GetFont(Font)->Print3d(Text.c_str());
        else
            assert(false);
    }

private:
    void Initialize()
    {
        Color[0] = 255;
        Color[1] = 0;
        Color[2] = 0;
        Color[3] = 255;
    }
};

#endif
