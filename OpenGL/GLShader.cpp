#include "GLShader.h"

#include <iostream>
#include <fstream>
#include "../Utils/Debug.h"

GLShader *GLShader::currShader  = 0;
GLenum    GLShader::currProgram = 0;
int       GLShader::lights      = 0;
int       GLShader::textures    = 0;

void GLShader::Load(const char *vShaderFile, const char *fShaderFile)
{
    Unload();

    std::ifstream in;

    if (vShaderFile) {
        vertexShaderFile = strdup(vShaderFile);
        in.open(vShaderFile, std::ios::in);
        if (in.is_open()) {
            in.seekg( 0, std::ios::end );
            int width = in.tellg();
            in.seekg( 0, std::ios::beg );

            vertexShaderSrc = new char[width+1];
            in.read(vertexShaderSrc, width);
            width = in.gcount();
            vertexShaderSrc[width] = 0;
            in.close();
        }
        in.clear();
    }

    if (fShaderFile) {
        fragmentShaderFile = strdup(fShaderFile);
        in.open(fShaderFile, std::ios::in);
        if (in.is_open()) {
            in.seekg( 0, std::ios::end );
            int width = in.tellg();
            in.seekg( 0, std::ios::beg );

            fragmentShaderSrc = new char[width+1];
            in.read(fragmentShaderSrc, width);
            width = in.gcount();
            fragmentShaderSrc[width] = 0;
            in.close();
        }
    }
}

GLenum GLShader::Initialize()
{
    assert(!program);
    assert(!vertex_shader);
    assert(!fragment_shader);
    if (CheckForGLError("Pre create GLSL program")) {}

    if (!agl_ShaderObjectsLoaded || !agl_VertexShaderLoaded || !agl_FragmentShaderLoaded)
    {
        if (!agl_ShaderObjectsLoaded)  LOG("WARNING: agl_ShaderObjectsLoaded is not supported\n");
        if (!agl_VertexShaderLoaded)   LOG("WARNING: agl_VertexShaderLoaded is not supported\n");
        if (!agl_FragmentShaderLoaded) LOG("WARNING: agl_FragmentShaderLoaded is not supported\n");
        return 0;
    }

    if (!vertexShaderSrc && !fragmentShaderSrc)
    {
        LOG("WARNING: shader source not found\n");
        return 0;
    }

    do {
        // Create Shader And Program Objects
        program = glCreateProgramObjectARB();
        if (CheckForGLError("Cannot create GLSL program")) {
            program = 0;
            break;
        }

        if (vertexShaderSrc) {
            vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
            if (CheckForGLError("Cannot create vertex shader")) {
                Terminate();
                break;
            }
            glShaderSourceARB(vertex_shader, 1, (const char **)&vertexShaderSrc, NULL);
            glCompileShaderARB(vertex_shader);
            glAttachObjectARB(program, vertex_shader);
        }

        if (fragmentShaderSrc) {
            fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
            if (CheckForGLError("Cannot create fragment shader")) {
                Terminate();
                break;
            }
            glShaderSourceARB(fragment_shader, 1, (const char **)&fragmentShaderSrc, NULL);
            glCompileShaderARB(fragment_shader);
            glAttachObjectARB(program, fragment_shader);
        }
    }
    while(false);

    if (program) {
        // Link The Program Object
        glLinkProgramARB(program);

        int blen = 0;
        int slen = 0;
        glGetObjectParameterivARB(program, GL_OBJECT_INFO_LOG_LENGTH_ARB , &blen);

        if (blen > 1)
        {
            char *linker_log;
            if ((linker_log = new GLcharARB[blen]) == NULL)
                LOG("ERROR: Could not allocate compiler_log buffer\n");

            glGetInfoLogARB(program, blen, &slen, linker_log);
            linker_log[slen-1] = 0;
            LOG("%s,%s\n%s", vertexShaderFile, fragmentShaderFile, linker_log);

            if (strstr(linker_log, "error") || strstr(linker_log, "failed"))
            {
                delete[] linker_log;
                Terminate();
                return 0;
            }
            delete[] linker_log;
        }

        uTexturing = glGetUniformLocationARB(program, "texturing");
        uLighting  = glGetUniformLocationARB(program, "lighting");
    }

    return program;
}
