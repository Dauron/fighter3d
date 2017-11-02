#include "ISelectionProvider.h"

#include "../../Config.h"
#include "Shader.h"
#include <GL/glu.h>

using namespace Graphics::OGL;

std::vector<xDWORD> * ISelectionProvider:: Select(const Math::Cameras::Camera &Camera,
                                                  int X, int Y, int W, int H)
{
    if (W == 0) W = 1;
    if (H == 0) H = 1;

    int nbRecords = 0;
    GLuint objectID = xDWORD_MAX;

    //Calculate select buffer capacity and allocate data if necessary
    int capacity = 4*CountSelectable()*1; //Each object take in maximium : 4 * name stack depth
    GLuint *selectBuffer = new GLuint[capacity];

    //Send select buffer to OpenGl and use select mode to track object hits
    glSelectBuffer(capacity, selectBuffer);
    glRenderMode(GL_SELECT);
    State::RenderingSelection = true;
    Shader::SetLightType(xLight_NONE);
    Shader::EnableTexturing(xState_Disable);

    //Select the projection matrix
    glViewport(Camera.FOV.ViewportLeft, Camera.FOV.ViewportTop, Camera.FOV.ViewportWidth, Camera.FOV.ViewportHeight);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    {
        GLint viewport[] = { (GLint)Camera.FOV.ViewportLeft,  (GLint)Camera.FOV.ViewportTop,
                             (GLint)Camera.FOV.ViewportWidth, (GLint)Camera.FOV.ViewportHeight };
        //Restrict drawing in our picking region.
        gluPickMatrix(X+W/2, Y-H/2, W, H, viewport);
        //Defines the viewing volume
        glMultMatrixf(&Camera.FOV.MX_Projection_Get().x0);
        //Select the modelview matrix for the drawing
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(&Camera.MX_WorldToView_Get().x0);
        //Draw the scene
        glInitNames();
        glPushName(0);
        RenderSelect(Camera.FOV);
        //Return to render mode, glRenderMode returns the number of hits (only because GL_SELECT was selected before)
        nbRecords = glRenderMode(GL_RENDER);
        State::RenderingSelection = false;
    }
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    Shader::EnableTexturing(xState_Enable);

    if(nbRecords <= 0)
    {
        delete[] selectBuffer;
        return NULL; //Don't pick any objects
    }

    /*
     * Select buffer
     * -------------
     * The select buffer is a list of nbRecords records.
     * Each records is composed of :
     *  1st int  : depth of the name stack
     *  2nd int  : minimum depth value
     *  3rd int  : maximum depth value
     *  x int(s) : list of name (number is defined in the 1st integer))
     */
    std::vector<xDWORD> *res = new std::vector<xDWORD>();

    GLuint *record = selectBuffer, minDepth = (GLuint)-1; // (GLuint)-1 = MAXUINT
    do
    {
        if(minDepth == (GLuint)-1 || minDepth > record[2])
        {
            if (minDepth != (GLuint)-1)
                res->push_back(objectID);
            minDepth = record[1];
            objectID = record[3];
        }
        else
            res->push_back(record[3]);
        record += 4;
    }
    while (--nbRecords);

    if (minDepth != (GLuint)-1)
        res->push_back(objectID);

    delete[] selectBuffer;
    return res;
}
