#include "DeferredFBO.h"

#include <cassert>
#include "../../Utils/Debug.h"
#include "Shader.h"

using namespace Graphics::OGL;

bool DeferredFBO:: Create ( int width, int height )
{
    glGenFramebuffersEXT( 1, &FBO_deferred);
    glGenFramebuffersEXT( 1, &FBO_lightAccum);
    glGenFramebuffersEXT( 1, &FBO_filter1);
    glGenFramebuffersEXT( 1, &FBO_filter2);

    glGenTextures( 1, &TEX_depth_stencil);
    glGenTextures( 1, &TEX_deferred_position);
    glGenTextures( 1, &TEX_deferred_normal);
    glGenTextures( 1, &TEX_deferred_material);
    glGenTextures( 1, &TEX_light_diffuse);
    glGenTextures( 1, &TEX_light_specular);
    glGenTextures( 1, &TEX_filter1);
    glGenTextures( 1, &TEX_filter2);

    return Resize(width, height);
}

bool DeferredFBO:: Resize ( int width, int height )
{
    assert(IsValid());

    Width  = width;
    Height = height;

    //
    // Create the deferred shading FBO (position, normal, material, depth)
    //

    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, FBO_deferred);

    glBindTexture  ( GL_TEXTURE_2D, TEX_deferred_position);
    glTexImage2D   ( GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, TEX_deferred_position, 0);

    glBindTexture  ( GL_TEXTURE_2D, TEX_deferred_normal);
    glTexImage2D   ( GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, TEX_deferred_normal, 0);

    glBindTexture  ( GL_TEXTURE_2D, TEX_deferred_material);
    glTexImage2D   ( GL_TEXTURE_2D, 0, /*GL_RGBA8*/ GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, TEX_deferred_material, 0);

    glBindTexture  ( GL_TEXTURE_2D, TEX_depth_stencil);
    glTexImage2D   ( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8_EXT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,   GL_TEXTURE_2D, TEX_depth_stencil, 0);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, TEX_depth_stencil, 0);

    glBindTexture( GL_TEXTURE_2D, 0);

    if ( glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT )
        LOG(1, "DeferredFBO::Create : bad frame buffer config");

    //
    // Light accum FBO
    //

    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, FBO_lightAccum);

    glBindTexture  ( GL_TEXTURE_2D, TEX_light_diffuse);
    glTexImage2D   ( GL_TEXTURE_2D, 0, /*GL_R11F_G11F_B10F_EXT*/GL_RGB16F_ARB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glGenerateMipmapEXT( GL_TEXTURE_2D);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, TEX_light_diffuse, 0);

    glBindTexture  ( GL_TEXTURE_2D, TEX_light_specular);
    glTexImage2D   ( GL_TEXTURE_2D, 0, /*GL_R11F_G11F_B10F_EXT*/GL_RGB16F_ARB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glGenerateMipmapEXT( GL_TEXTURE_2D);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, TEX_light_specular, 0);

    //use the same depth buffer as the deferred shading
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,   GL_TEXTURE_2D, TEX_depth_stencil, 0);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, TEX_depth_stencil, 0);

    glBindTexture( GL_TEXTURE_2D, 0);

    if ( glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT )
        LOG(1, "DeferredFBO::Create : bad frame buffer config");

    //
    // Filter FBO 1
    //

    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, FBO_filter1);

    glBindTexture  ( GL_TEXTURE_2D, TEX_filter1);
    glTexImage2D   ( GL_TEXTURE_2D, 0, GL_RGB16F_ARB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, TEX_filter1, 0);

    //use the same depth buffer as the deferred shading
    //glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,   GL_TEXTURE_2D, TEX_depth_stencil, 0);
    //glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, TEX_depth_stencil, 0);

    glBindTexture( GL_TEXTURE_2D, 0);

    if ( glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT )
        LOG(1, "DeferredFBO::Create : bad frame buffer config");

    //
    // Filter FBO 2
    //

    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, FBO_filter2);

    glBindTexture  ( GL_TEXTURE_2D, TEX_filter2);
    glTexImage2D   ( GL_TEXTURE_2D, 0, GL_RGB16F_ARB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, TEX_filter2, 0);

    //use the same depth buffer as the deferred shading
    //glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,   GL_TEXTURE_2D, TEX_depth_stencil, 0);
    //glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, TEX_depth_stencil, 0);

    glBindTexture( GL_TEXTURE_2D, 0);

    if ( glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT )
        LOG(1, "DeferredFBO::Create : bad frame buffer config");

    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0);

    return true;
}

void DeferredFBO:: Destroy()
{
    if (IsValid())
    {
        glDeleteFramebuffersEXT( 1, &FBO_deferred);
        glDeleteFramebuffersEXT( 1, &FBO_lightAccum);
        glDeleteFramebuffersEXT( 1, &FBO_filter1);
        glDeleteFramebuffersEXT( 1, &FBO_filter2);
        glDeleteTextures( 1, &TEX_depth_stencil);
        glDeleteTextures( 1, &TEX_deferred_position);
        glDeleteTextures( 1, &TEX_deferred_normal);
        glDeleteTextures( 1, &TEX_deferred_material);
        glDeleteTextures( 1, &TEX_light_diffuse);
        glDeleteTextures( 1, &TEX_light_specular);
        glDeleteTextures( 1, &TEX_filter1);
        glDeleteTextures( 1, &TEX_filter2);
        Clear();
    }
}

void DeferredFBO:: Stage1_MRT(xColor color)
{
    GLenum bufs[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT};

    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, FBO_deferred);
    glViewport( 0, 0, Width, Height);
    glColorMask(1,1,1,1);

    glDrawBuffers( 1, bufs + 2);
    glClearColor( color.r, color.g, color.b, 0.f );
    glClear( GL_COLOR_BUFFER_BIT );
    glDrawBuffers( 1, bufs + 1);
    glClearColor( 0.5, 0.5, 0.5, 0.0);
    glClear( GL_COLOR_BUFFER_BIT );
    glDrawBuffers( 1, bufs + 0);
    glClearColor( 0.0, 0.0, 0.0, 0.0);
    glClear( GL_COLOR_BUFFER_BIT );

    glDrawBuffers( 3, bufs);
    glClear( GL_DEPTH_BUFFER_BIT );

    Shader::StartDeferred(xDPS_Stage1_MRT);
}

void DeferredFBO:: Stage2_PreLights()
{
    GLenum bufs[]   = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO_lightAccum);
    glDrawBuffers       (2, bufs);

    glViewport  (0, 0, Width, Height);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear     (GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture  (GL_TEXTURE_2D, TEX_deferred_position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture  (GL_TEXTURE_2D, TEX_deferred_normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture  (GL_TEXTURE_2D, TEX_deferred_material);

    glBlendFunc (GL_ONE, GL_ONE);
    glEnable    (GL_BLEND);
    //glDepthMask (GL_FALSE);
    //glEnable    (GL_DEPTH_TEST);
    glDisable   (GL_DEPTH_TEST);

    /// here go lights
}

void DeferredFBO:: Stage2_InfiniteLight(xLight &light)
{
    LightSet_GL(light);
    Shader::StartDeferred(xDPS_Stage2_Infinite);
    ShaderDeferredS2 *currShader = (ShaderDeferredS2 *)Shader::currShader;
    glUniform1iARB( currShader->uTEX_position, 0);
    glUniform1iARB( currShader->uTEX_normal,   1);
    glUniform1iARB( currShader->uTEX_material, 2);

    glBegin(GL_QUADS);
    {
        glVertex2f(0.f, 0.f);
        glVertex2f(1.f, 0.f);
        glVertex2f(1.f, 1.f);
        glVertex2f(0.f, 1.f);
    }
    glEnd();
}

void DeferredFBO:: Stage2_PointLight(xLight &light, Math::Cameras::Camera &camera)
{
    xPoint3 P_light = camera.MX_WorldToView_Get().preTransformP(light.position);
    if (P_light.z > light.radius) return;

    xPoint3 P_light_corner1;
    P_light_corner1.init (P_light.x - light.radius, P_light.y - light.radius, P_light.z);
    P_light         = camera.FOV.MX_Projection_Get().preTransformV(P_light);
    P_light_corner1 = camera.FOV.MX_Projection_Get().preTransformV(P_light_corner1);

    xVector2 NW_shift; NW_shift.init(fabs(P_light.x - P_light_corner1.x), fabs(P_light.y - P_light_corner1.y));
    NW_shift        *= 0.5f;
    P_light.vector2 *= 0.5f;
    P_light.vector2 += 0.5f;

    xPoint2 P_light1, P_light2, P_light3, P_light4;
    P_light1.init (max(0.f, P_light.x - NW_shift.x), max(0.f, P_light.y - NW_shift.y));
    P_light2.init (min(1.f, P_light.x + NW_shift.x), max(0.f, P_light.y - NW_shift.y));
    P_light3.init (min(1.f, P_light.x + NW_shift.x), min(1.f, P_light.y + NW_shift.y));
    P_light4.init (max(0.f, P_light.x - NW_shift.x), min(1.f, P_light.y + NW_shift.y));

    LightSet_GL(light);
    Shader::StartDeferred(xDPS_Stage2_Point);
    ShaderDeferredS2 *currShader = (ShaderDeferredS2 *)Shader::currShader;
    glUniform1iARB( currShader->uTEX_position, 0);
    glUniform1iARB( currShader->uTEX_normal,   1);
    glUniform1iARB( currShader->uTEX_material, 2);

    glBegin(GL_QUADS);
    {
        glVertex2fv(P_light1.xy);
        glVertex2fv(P_light2.xy);
        glVertex2fv(P_light3.xy);
        glVertex2fv(P_light4.xy);
    }
    glEnd();
}

void DeferredFBO:: Stage2_PostLights()
{
    //glDisable   (GL_DEPTH_TEST);
    //glDepthMask (GL_TRUE);
    glDisable   (GL_BLEND);

    //glActiveTexture( GL_TEXTURE2);
    glBindTexture  (GL_TEXTURE_2D, 0);
    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture  (GL_TEXTURE_2D, 0);
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture  (GL_TEXTURE_2D, 0);
}

void DeferredFBO:: Stage3_Join()
{
    GLenum bufs[] = { GL_COLOR_ATTACHMENT0_EXT };
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO_current = FBO_filter1);
    glDrawBuffers       (1, bufs);

    sampleWidth  = (float)Width;
    sampleHeight = (float)Height;
    texScale     = 1.f;
    glViewport  (0, 0, Width, Height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture  (GL_TEXTURE_2D, TEX_light_diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture  (GL_TEXTURE_2D, TEX_light_specular);

    Shader::StartDeferred(xDPS_Stage3_Join);
    ShaderDeferredS3 *currShader = (ShaderDeferredS3 *)Shader::currShader;
    glUniform1iARB( currShader->uTEX_diffuse,  0);
    glUniform1iARB( currShader->uTEX_specular, 1);

    glBegin(GL_QUADS);
    {
        glVertex2f(0.f, 0.f);
        glVertex2f(1.f, 0.f);
        glVertex2f(1.f, 1.f);
        glVertex2f(0.f, 1.f);
    }
    glEnd();

    //glActiveTexture(GL_TEXTURE1);
    glBindTexture  (GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture  (GL_TEXTURE_2D, 0);
}

void DeferredFBO:: Stage4_PreFilters()
{
    glMatrixMode  (GL_PROJECTION);
    glLoadIdentity();
    glOrtho       (0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
    glMatrixMode  (GL_MODELVIEW);
    glLoadIdentity();
    glColor3ub    (255,255,255);
}

void DeferredFBO:: Stage4_ConvolutionFilter(xFLOAT Mask[9])
{
    GLenum bufs[] = { GL_COLOR_ATTACHMENT0_EXT };

    glActiveTexture(GL_TEXTURE0);

    if (FBO_current == FBO_filter1)
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO_current = FBO_filter2);
        glBindTexture  (GL_TEXTURE_2D, TEX_filter1);
    }
    else
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO_current = FBO_filter1);
        glBindTexture  (GL_TEXTURE_2D, TEX_filter2);
    }

    glDrawBuffers (1, bufs);
    glViewport    (0, 0, Width, Height);

    Shader::StartDeferred(xDPS_Stage4_Convolution);
    ShaderConvolution *currShader = (ShaderConvolution *)Shader::currShader;
    glUniform1iARB       (currShader->uTEX_image, 0);
    glUniformMatrix3fvARB(currShader->uMask, 1, false, Mask);
    glUniform2fvARB      (currShader->uTexOffset, 1, xVector2::Create(1.f/Width, 1.f/Height).xy);

    glBegin(GL_QUADS);
    {
        glVertex2f(0.f,      0.f);
        glVertex2f(texScale, 0.f);
        glVertex2f(texScale, texScale);
        glVertex2f(0.f,      texScale);
    }
    glEnd();
}

void DeferredFBO:: Stage4_Resize(float scale)
{
    if (scale == 1.f) return;

    GLenum bufs[] = { GL_COLOR_ATTACHMENT0_EXT };

    glActiveTexture(GL_TEXTURE0);
    glEnable       (GL_TEXTURE_2D);
    if (FBO_current == FBO_filter1)
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO_current = FBO_filter2);
        glBindTexture  (GL_TEXTURE_2D, TEX_filter1);
    }
    else
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO_current = FBO_filter1);
        glBindTexture  (GL_TEXTURE_2D, TEX_filter2);
    }

    sampleWidth  *= scale;
    sampleHeight *= scale;

    glDrawBuffers (1, bufs);
    glViewport    (0, 0, (int)sampleWidth, (int)sampleHeight);

    Shader::StopDeferred();

    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.f,      0.f);      glVertex2f(0.f, 0.f);
        glTexCoord2f(texScale, 0.f);      glVertex2f(1.f, 0.f);
        glTexCoord2f(texScale, texScale); glVertex2f(1.f, 1.f);
        glTexCoord2f(0.f,      texScale); glVertex2f(0.f, 1.f);
    }
    glEnd();

    texScale *= scale;
}

void DeferredFBO:: Stage4_Tint(xColor color, float treshold)
{
    GLenum bufs[] = { GL_COLOR_ATTACHMENT0_EXT };

    glActiveTexture(GL_TEXTURE0);
    glEnable       (GL_TEXTURE_2D);
    if (FBO_current == FBO_filter1)
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO_current = FBO_filter2);
        glBindTexture  (GL_TEXTURE_2D, TEX_filter1);
    }
    else
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO_current = FBO_filter1);
        glBindTexture  (GL_TEXTURE_2D, TEX_filter2);
    }

    glDrawBuffers (1, bufs);

    glViewport    (0, 0, Width, Height);

    Shader::StartDeferred(xDPS_Stage4_Tint);
    ShaderDeferredS4 *currShader = (ShaderDeferredS4 *)Shader::currShader;
    glUniform1iARB (currShader->uTEX_image, 0);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   color.xyzw);
    glMaterialf(GL_FRONT,  GL_SHININESS, treshold);
    glBegin(GL_QUADS);
    {
        glVertex2f(0.f,      0.f);
        glVertex2f(texScale, 0.f);
        glVertex2f(texScale, texScale);
        glVertex2f(0.f,      texScale);
    }
    glEnd();
}

void DeferredFBO:: Stage8_Fin()
{
    Shader::StopDeferred();

    GLenum standardBufs[] = { GL_BACK_LEFT };
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glDrawBuffers       (1, standardBufs);
    glViewport          (0, 0, Width, Height);

    glActiveTexture(GL_TEXTURE0);
    glEnable       (GL_TEXTURE_2D);
    if (FBO_current == FBO_filter1)
        glBindTexture  (GL_TEXTURE_2D, TEX_filter1);
    else
        glBindTexture  (GL_TEXTURE_2D, TEX_filter2);

    glMatrixMode  (GL_PROJECTION);
    glLoadIdentity();
    glOrtho       (0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
    glMatrixMode  (GL_MODELVIEW);
    glLoadIdentity();

    glColor3ub(255,255,255);
    glBegin( GL_QUADS);
    {
        glTexCoord2f(0.f, 0.f); glVertex2f  (0.f, 0.f);
        glTexCoord2f(1.f, 0.f); glVertex2f  (1.f, 0.f);
        glTexCoord2f(1.f, 1.f); glVertex2f  (1.f, 1.f);
        glTexCoord2f(0.f, 1.f); glVertex2f  (0.f, 1.f);
    }
    glEnd();

    glDisable     (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, 0);
}

void DeferredFBO:: Stage9_Test()
{
    // Test MRT
    glMatrixMode  (GL_PROJECTION);
    glLoadIdentity();
    glOrtho       (0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
    glMatrixMode  (GL_MODELVIEW);
    glLoadIdentity();

    glColor3ub(255,255,255);

    glActiveTexture(GL_TEXTURE0);
    glEnable       (GL_TEXTURE_2D);

    glBindTexture  (GL_TEXTURE_2D, TEX_light_diffuse);
    glBegin( GL_QUADS);
    {
        glTexCoord2f(0.0f,  0.f);
        glVertex2f  (0.f,   0.5f);
        glTexCoord2f(1.f,   0.f);
        glVertex2f  (0.25f, 0.5f);
        glTexCoord2f(1.f,   1.f);
        glVertex2f  (0.25f, 0.75f);
        glTexCoord2f(0.f,   1.f);
        glVertex2f  (0.f,   0.75f);
    }
    glEnd();

    glBindTexture (GL_TEXTURE_2D, TEX_deferred_normal);
    glBegin( GL_QUADS);
    {
        glTexCoord2f(0.0f,  0.f);
        glVertex2f  (0.f,   0.25f);
        glTexCoord2f(1.f,   0.f);
        glVertex2f  (0.25f, 0.25f);
        glTexCoord2f(1.f,   1.f);
        glVertex2f  (0.25f, 0.5f);
        glTexCoord2f(0.f,   1.f);
        glVertex2f  (0.f,   0.5f);
    }
    glEnd();

    glBindTexture (GL_TEXTURE_2D, TEX_deferred_material);
    glBegin( GL_QUADS);
    {
        glTexCoord2f(0.0f,  0.f);
        glVertex2f  (0.f,   0.f);
        glTexCoord2f(1.f,   0.f);
        glVertex2f  (0.25f, 0.f);
        glTexCoord2f(1.f,   1.f);
        glVertex2f  (0.25f, 0.25f);
        glTexCoord2f(0.f,   1.f);
        glVertex2f  (0.f,   0.25f);
    }
    glEnd();

    glDisable     (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, 0);
}
