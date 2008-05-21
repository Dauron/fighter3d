#include "../AppFramework/System.h"
#include "../OpenGL/GLShader.h"
#include <GL/gl.h>

void setMaterials()
{
    GLfloat material_diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat material_specular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat material_shininess[] = { 10.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);

    // Set the GL_AMBIENT_AND_DIFFUSE color state variable to be the
    // one referred to by all following calls to glColor
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
}

void setLights()
{
    if (g_EnableLighting)
    {
        GLfloat light_global_amb_color[]  = { 0.2f, 0.2f, 0.2f, 1.0f };

        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_global_amb_color);
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // GL_FALSE = infinite viewpoint, GL_TRUE = locale viewpoint
        glEnable(GL_LIGHTING);

        // light 0
        GLfloat light_position[]   = { 0.0f, 0.0f, 10.0f, 1.0f }; // w=0 - directional, w=1 - positional
        GLfloat light_amb_color[]  = { 0.09f, 0.07f, 0.0f, 1.0f };
        GLfloat light_dif_color[]  = { 0.9f, 0.7f, 0.0f, 1.0f };
        GLfloat light_spec_color[] = { 0.9f, 0.7f, 0.0f, 1.0f };

        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_AMBIENT,  light_amb_color);  // environment
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_dif_color);  // direct light
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_spec_color); // light on mirrors/metal

        // rozpraszanie siê œwiat³a
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  1.0f);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.004f);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0008f);

        //spot cone - for positional lights
        GLfloat spot_direction[] = { 0.0f, 0.0f, -1.0f };
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 45.0f);
        glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 1); // spot attenuation 0-no attenuation 128-max

        glEnable(GL_LIGHT0);

        // light 1
        GLfloat light_position1[]   = { 10.0f, -5.0f, 5.0f, 1.0f }; // w=0 - directional, w=1 - positional
        GLfloat light_amb_color1[]  = { 0.1f, 0.0f, 0.0f, 1.0f };
        GLfloat light_dif_color1[]  = { 1.0f, 0.0f, 0.0f, 1.0f };
        GLfloat light_spec_color1[] = { 1.0f, 0.0f, 0.0f, 1.0f };

        glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
        glLightfv(GL_LIGHT1, GL_AMBIENT,  light_amb_color1);  // environment
        glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_dif_color1);  // direct light
        glLightfv(GL_LIGHT1, GL_SPECULAR, light_spec_color1); // light on mirrors/metal

        // rozpraszanie siê œwiat³a
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION,  1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.004f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0008f);

        //spot cone - for positional lights
        GLfloat spot_direction1[] = { 0.0f, 0.0f, -1.0f };
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction1);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 45.0f);
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 1); // spot attenuation 0-no attenuation 128-max

        glEnable(GL_LIGHT1);

        // light 2
        //GLfloat light_position2[]  = { -10.0f, -10.0f, 10.0f, 1.0f }; // w=0 - directional, w=1 - positional
        GLfloat light_position2[]   = { 0.0f, 0.0f, 100.0f, 1.0f }; // w=0 - directional, w=1 - positional
        GLfloat light_amb_color2[]  = { 0.04f, 0.04f, 0.04f, 1.0f };
        GLfloat light_dif_color2[]  = { 0.4f, 0.4f, 0.4f, 1.0f };
        GLfloat light_spec_color2[] = { 0.4f, 0.4f, 0.4f, 1.0f };

        glLightfv(GL_LIGHT2, GL_POSITION, light_position2);
        glLightfv(GL_LIGHT2, GL_AMBIENT,  light_amb_color2);  // environment
        glLightfv(GL_LIGHT2, GL_DIFFUSE,  light_dif_color2);  // direct light
        glLightfv(GL_LIGHT2, GL_SPECULAR, light_spec_color2); // light on mirrors/metal

        // rozpraszanie siê œwiat³a
        glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION,  1.0f);
        glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.000f);
        glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.0000f);

        //spot cone - for positional lights
        GLfloat spot_direction2[] = { 0.0f, 0.0f, -1.0f };
        glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_direction2);
        glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 45.0f);
        glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 1); // spot attenuation 0-no attenuation 128-max

        glEnable(GL_LIGHT2);

        GLShader::EnableLighting(3);
    }
    else
    {
        glDisable(GL_LIGHTING);
        GLShader::EnableLighting(0);
    }
}

void setFog()
{
    //Color of the fog : grey fog
    float fogColor[] = {0.9f, 0.9f, 0.9f, 1.0f};
    glFogfv(GL_FOG_COLOR, fogColor);
    /*
     * Select the fog mode
     *     GL_EXP, GL_EXP2 or GL_LINEAR
     */
    //glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogi(GL_FOG_MODE, GL_EXP2);
    /*
     * Density of the fog
     *     ONLY used by the following fog modes : GL_EXP, GL_EXP2
     */
    glFogf(GL_FOG_DENSITY, 0.1f);
    /*
     * Start of the fog (object begin to mix with the fog) and end of the fog (objects disappear)
     * The value
     *     ONLY used by the fog mode : GL_LINEAR
     */
    glFogf(GL_FOG_START, 0.0f);
    glFogf(GL_FOG_END, 100.0f);
    /*
     * Precision of the fog calculation (per pixel or per vertex)
     *     GL_DONT_CARE, GL_NICEST (per pixel calculation) or GL_FASTEST (per vertex calculation)
     */
    glHint(GL_FOG_HINT, GL_NICEST);

    //enable the fog mode
    glEnable(GL_FOG);

    //Clearing color equal to the fog color
    glClearColor(fogColor[0], fogColor[1], fogColor[2], 0.0f);
}
