#ifndef __incl_GLShader_h
#define __incl_GLShader_h     1

#include <cassert>
#include "../AppFramework/System.h"
#include <GL/gl.h>
#include "../GLExtensions/aGL_Extensions.h"
#include "../Math/xMath.h"
#include "../Utils/Debug.h"

class GLShader
{
    static GLShader *currShader;
    static GLenum    currProgram;
    static int lights;
    static int textures;

    char *vertexShaderSrc;
    char *fragmentShaderSrc;

    GLenum program;
    GLenum vertex_shader;
    GLenum fragment_shader;

    int uTexturing;
    int uLighting;

public:

    GLShader()
    {
        program           = 0;
        vertex_shader     = 0;
        fragment_shader   = 0;
        vertexShaderSrc   = 0;
        fragmentShaderSrc = 0;

        uTexturing = 0;
        uLighting  = 0;
    }
    ~GLShader()
    {
        Terminate();
        Unload();
    }

    void Load(const char *vShaderFile, const char *fShaderFile);
    void Unload()
    {
        if (vertexShaderSrc)   delete[] vertexShaderSrc;
        vertexShaderSrc = NULL;
        if (fragmentShaderSrc) delete[] fragmentShaderSrc;
        fragmentShaderSrc = NULL;
    }

    GLenum IsInitialized() { return program; }
    GLenum Initialize();
    void Terminate()
    {
        if (vertex_shader)
            glDeleteObjectARB(vertex_shader);
        if (fragment_shader)
            glDeleteObjectARB(fragment_shader);
        if (program)
            glDeleteObjectARB(program);
        Invalidate();
    }
    void Invalidate()
    {
        program = vertex_shader = fragment_shader = 0;
        uTexturing = uLighting = 0;
    }

    bool IsCurrent() { return program == currProgram; }
    GLShader *Start()
    {
        assert(program);
        assert(!IsCurrent());
        // Use The Program Object Instead Of Fixed Function OpenGL
        GLShader *prevShader = currShader;
        currShader = this;
        glUseProgramObjectARB(currProgram = program);
        glUniform1iARB(uTexturing, textures);
        glUniform1iARB(uLighting, lights);
        return prevShader;
    }
    void Suspend()
    {
        assert(program);
        assert(IsCurrent());
        // Use The Fixed Function OpenGL
        glUseProgramObjectARB(currProgram = 0);
        currShader = NULL;
    }

    void SetUniform(const char *paramName, int val)
    {
        if (program) {
            int ptr = glGetUniformLocationARB(program, paramName);
            glUniform1iARB(ptr, val);
        }
    }

    static void EnableTexturing(int val) // 0, 1, 2
    {
        textures = val;
        if (currShader)
            glUniform1iARB(currShader->uTexturing, textures);
    }

    static void EnableLighting(int val) // 0, 1, 2, 3
    {
        lights = val;
        if (currShader)
            glUniform1iARB(currShader->uLighting, lights);
    }
};

#endif
