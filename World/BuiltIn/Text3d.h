#ifndef __incl_Text3d_h
#define __incl_Text3d_h

#include "../Object3D.h"
#include "../../OpenGL/Fonts/FontMgr.h"
#include <string>

class Text3d : public Object3D
{
public:
    Text3d (std::string text, HFont font) : Object3D(), Text(text), Font(font)
    { Initialize(); }
    Text3d (std::string text, HFont font, GLfloat x, GLfloat y, GLfloat z)
      : Object3D(x,z,y), Text(text), Font(font)
    { Initialize(); }
    Text3d (std::string text, HFont font, GLfloat x, GLfloat y, GLfloat z,
         GLfloat rotX, GLfloat rotY, GLfloat rotZ)
      : Object3D(x,z,y, rotX,rotZ,rotY), Text(text), Font(font)
    { Initialize(); }

    virtual void SetPosition(GLfloat x, GLfloat y, GLfloat z)
    {
        Object3D::SetPosition(x,z,y);
    }
    virtual GLfloat *GetPosition()
    {
        GLfloat *res = Object3D::GetPosition();
        GLfloat tmp = res[3];
        res[2] = res[3];
        res[3] = tmp;
        return res;
    }

    virtual void SetRotation(GLfloat rotX, GLfloat rotY, GLfloat rotZ)
    {
        Object3D::SetRotation(rotX,rotZ,rotY);
    }
    virtual GLfloat *GetRotation()
    {
        GLfloat *res = Object3D::GetRotation();
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
            g_FontMgr.GetFont(Font)->Print3d(Text.data());
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
