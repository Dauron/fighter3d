#ifndef __incl_Crate_h
#define __incl_Crate_h

#include "../Object3D.h"
#include "../../OpenGL/Textures/TextureMgr.h"

class Crate : public Object3D
{
public:
      Crate () : Object3D() {}
      Crate (GLfloat x, GLfloat y, GLfloat z)
          : Object3D(x,y,z+0.64f) {}
      Crate (GLfloat x, GLfloat y, GLfloat z,
             GLfloat rotX, GLfloat rotY, GLfloat rotZ)
          : Object3D(x,y,z+0.64f, rotX,rotY,rotZ) {}

      ~Crate ()
      {
          Finalize();
      }

      virtual void SetPosition(GLfloat x, GLfloat y, GLfloat z)
      {
          Object3D::SetPosition(x,y,z+0.64f);
      }
      virtual GLfloat *GetPosition()
      {
          GLfloat *res = Object3D::GetPosition();
          res[2] -= 0.64f;
          return res;
      }

private:
    void Finalize ()
    {
        if (TextureMgr::GetSingletonPtr())
            g_TextureMgr.DeleteTexture(crateHandle);
        crateHandle = HTexture();
    }

protected:
    HTexture crateHandle;

    void BindTexture()
    {
        if (crateHandle.IsNull())
            crateHandle = g_TextureMgr.GetTexture("Data\\crate.bmp");
        g_TextureMgr.BindTexture(crateHandle);
        g_TextureMgr.EnableTextures();
    }

    void RenderObject()
    {
        BindTexture();
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        // Draw the sides of the cube
        glBegin(GL_QUAD_STRIP);
            glNormal3d(0.0f, 1.0f, 0.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.64f, 0.64f, -0.64f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.64f, 0.64f, 0.64f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(0.64f, 0.64f, -0.64f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(0.64f, 0.64f, 0.64f);

            glNormal3d(1.0f, 0.0f, 0.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(0.64f, -0.64f, -0.64f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(0.64f, -0.64f, 0.64f);

            glNormal3d(0.0f, -1.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.64f, -0.64f, -0.64f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.64f, -0.64f, 0.64f);

            glNormal3d(-1.0f, 0.0f, 0.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.64f, 0.64f, -0.64f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.64f, 0.64f, 0.64f);
        glEnd();

        // Draw the top and bottom of the cube
        glBegin(GL_QUADS);
            glNormal3d(0.0f, 0.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.64f, 0.64f, -0.64f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(0.64f, 0.64f, -0.64f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(0.64f, -0.64f, -0.64f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.64f, -0.64f, -0.64f);

            glNormal3d(0.0f, 0.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.64f, -0.64f, 0.64f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(0.64f, -0.64f, 0.64f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(0.64f, 0.64f, 0.64f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.64f, 0.64f, 0.64f);
        glEnd();

        g_TextureMgr.DisableTextures();
    }
};

#endif
