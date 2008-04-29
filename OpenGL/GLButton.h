#ifndef __incl_GLButton_h
#define __incl_GLButton_h

#include "Utils.h"

class GLButton
{
    xWORD X,Y,X2,Y2;
    bool  mOver;

  public:
    int   Action;
    char *Text;
    bool  RadioBox;
    bool  Down;
    xColor Background;

    GLButton(const GLButton &button)
    {
        X  = button.X; X2 = button.X2;
        Y  = button.Y; Y2 = button.Y2;
        Action   = button.Action;
        mOver    = button.mOver;
        RadioBox = button.RadioBox;
        Down     = button.Down;
        Text     = strdup(button.Text);
        memcpy(&Background, &button.Background, sizeof(xColor));
    }
    GLButton(const char *text, xWORD x, xWORD y, xWORD w, xWORD h, int action, bool radioBox = false, bool down = false)
        : X(x), Y(y), X2(x+w), Y2(y+h), mOver(false), Action(action), RadioBox(radioBox), Down(down)
    {
        Text = strdup(text);
        Background.r = Background.g = Background.b = 0.3f; Background.a = 1.0f;
    }
    ~GLButton()
    {
        if (Text) delete[] Text;
        Text = NULL;
    }

    bool HitTest(xWORD x, xWORD y)
    {
        return (x >= X && y >= Y && x <= X2 && y <= Y2);
    }
    bool Hover(xWORD x, xWORD y)
    {
        return mOver = HitTest(x, y);
    }
    bool Click(xWORD x, xWORD y)
    {
        mOver = false;
        return HitTest(x, y);
    }

    void Render(const GLFont* pFont)
    {
        if (!Text) return;

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (Down)
        {
            if (RadioBox)
                glColor4f( 0.7f, 0.0f, 0.0f, 1.f );
            else
                glColor4f( 0.0f, 0.7f, 0.0f, 1.f );
        }
        else
        if (mOver)
            glColor4f( 0.7f, 0.7f, 0.0f, 1.f );
        else
            glColor4fv( Background.col );

        GLUtils::Rectangle(X, Y, X2, Y2);

        if (mOver)
            glColor4f( 0.f, 0.f, 0.f, 1.f );
        else
            glColor4f( 1.f, 1.f, 1.f, 1.f );
        pFont->PrintF(X+3.f, Y+3.f, 0.f, Text);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        GLUtils::Rectangle(X, Y, X2, Y2);
    }
};

#endif
