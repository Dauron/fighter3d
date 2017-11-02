#ifndef __incl_Graphics_OGL_Shader_h
#define __incl_Graphics_OGL_Shader_h

#include "ogl.h"
#include "../../Math/xMath.h"
#include "../../Math/xLight.h"

namespace Graphics { namespace OGL {

    struct ShaderProgram
    {
        static GLenum currProgram;

        GLenum program;
        bool   FL_invalid;

        GLenum vertex_shader;
        char *vertexShaderFile;
        char *vertexShaderSrc;

        GLenum fragment_shader;
        char *fragmentShaderFile;
        char *fragmentShaderSrc;

        ShaderProgram();
        virtual ~ShaderProgram() {}

        void Load(const char *vShaderFile, const char *fShaderFile);
        void Unload();

        GLenum IsCreated() { return program; }
        bool IsCurrent() { return program == currProgram; }

        virtual GLenum Create();
        virtual void   Destroy();
        virtual void   Invalidate()
        {
            program = vertex_shader = fragment_shader = 0; FL_invalid = true;
        }
    };

    struct ShaderDeferredS2 : public ShaderProgram
    {
        int uTEX_position;
        int uTEX_normal;
        int uTEX_material;

        ShaderDeferredS2();
        virtual ~ShaderDeferredS2() {}

        virtual GLenum Create();
    };

    struct ShaderDeferredS3 : public ShaderProgram
    {
        int uTEX_diffuse;
        int uTEX_specular;

        ShaderDeferredS3();
        virtual ~ShaderDeferredS3() {}

        virtual GLenum Create();
    };

    struct ShaderDeferredS4 : public ShaderProgram
    {
        int uTEX_image;

        ShaderDeferredS4();
        virtual ~ShaderDeferredS4() {}

        virtual GLenum Create();
    };

    struct ShaderConvolution : public ShaderDeferredS4
    {
        int uTexOffset;
        int uMask;

        ShaderConvolution();
        virtual ~ShaderConvolution() {}

        virtual GLenum Create();
    };

    struct ShaderTexBump : public ShaderProgram
    {
        int uTEX_color;
        int uTEX_bump;

        ShaderTexBump();
        virtual ~ShaderTexBump() {}

        virtual GLenum Create();
    };

    enum xState
    {
        xState_Disable = -2,
        xState_Enable  = -1,
        xState_Off     =  0,
        xState_On      =  1
    };

    enum xDeferredProgramStage
    {
        xDPS_Stage1_MRT         = 1,
        xDPS_Stage2_Infinite    = 2,
        xDPS_Stage2_Point       = 3,
        xDPS_Stage3_Join        = 4,
        xDPS_Stage4_Convolution = 5,
        xDPS_Stage4_Tint        = 6
    };

    class Shader
    {
        static bool   FL_deferredMRT;
        static GLuint TexDiffuseImg, TexDiffuseUnit;
        static GLuint TexBumpImg,    TexBumpUnit;

        static ShaderProgram     spDeferred_Stage1_MRT;
        static ShaderTexBump     spDeferred_Stage1_MRT_TexBump;
        static ShaderDeferredS2  spDeferred_Stage2_Infinite;
        static ShaderDeferredS2  spDeferred_Stage2_Point;
        static ShaderDeferredS3  spDeferred_Stage3_Join;
        static ShaderConvolution spDeferred_Stage4_Convolution;
        static ShaderDeferredS4  spDeferred_Stage4_Tint;

        Shader() {}
        ~Shader() {}

    public:
        static ShaderProgram *currShader;

        static void Load();
        static void Unload();
        static void CreateS();
        static void DestroyS();
        static void Invalidate();

        static bool StartDeferred(xDeferredProgramStage stage);
        static void StopDeferred();

        static bool Start();
        static void Suspend();

        static void UseDiffuseMap (GLuint ID_texture, GLuint ID_unit = 0)
        {
            TexDiffuseImg  = ID_texture;
            TexDiffuseUnit = ID_unit;
            glActiveTextureARB(GL_TEXTURE0 + ID_unit);
            glBindTexture(GL_TEXTURE_2D, ID_texture);
            if (ID_texture)
                glEnable(GL_TEXTURE_2D);
            else
                glDisable(GL_TEXTURE_2D);
        }
        static void UseBumpMap    (GLuint ID_texture, GLuint ID_unit = 1)
        {
            TexBumpImg  = ID_texture;
            TexBumpUnit = ID_unit;
            glActiveTextureARB(GL_TEXTURE0 + ID_unit);
            glBindTexture(GL_TEXTURE_2D, ID_texture);
            if (ID_texture)
                glEnable(GL_TEXTURE_2D);
            else
                glDisable(GL_TEXTURE_2D);
        }
    };

} } // namespace Graphics::OGL

#endif
