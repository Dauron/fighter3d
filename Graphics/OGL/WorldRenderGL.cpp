#include "WorldRenderGL.h"

#include "Utils.h"
#include "GLShader.h"
#include "Extensions/EXT_stencil_wrap.h"
#include "Extensions/EXT_stencil_two_side.h"
#include "Extensions/ARB_multisample.h"

void WorldRenderGL :: SetLight(xLight &light, bool t_Ambient, bool t_Diffuse, bool t_Specular)
{
    float light_off[4] = { 0.f, 0.f, 0.f, 0.f };

    // turn off ambient lighting
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_off);

    xVector4 position; position.init(light.position, light.type == xLight_INFINITE ? 0.f : 1.f);
    glLightfv(GL_LIGHT0, GL_POSITION, position.xyzw);

    glLightfv(GL_LIGHT0, GL_AMBIENT, t_Ambient ? light.ambient.col : light_off);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  t_Diffuse ? light.diffuse.col : light_off); // direct light
    glLightfv(GL_LIGHT0, GL_SPECULAR, t_Specular ? light.diffuse.col : light_off); // light on mirrors/metal

    // rozpraszanie si� �wiat�a
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  light.attenuationConst);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,    light.attenuationLinear);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, light.attenuationSquare);

    if (light.type == xLight_SPOT)
    {
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light.spotDirection.xyz);
        glLightf(GL_LIGHT0,  GL_SPOT_CUTOFF,    light.spotCutOff);
        glLightf(GL_LIGHT0,  GL_SPOT_EXPONENT,  light.spotAttenuation);
    }
    else
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0f);

    GLShader::SetLightType(light.type, t_Ambient, t_Diffuse, t_Specular);
}

void WorldRenderGL :: RenderWorld(World &world, Math::Cameras::CameraSet &cameraSet)
{
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // GL_FALSE = infinite viewpoint, GL_TRUE = locale viewpoint
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);    // GL_TRUE=two, GL_FALSE=one
    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_POLYGON_SMOOTH);                       // produces errors on many cards... use FSAA!
    glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2); glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4); glDisable(GL_LIGHT5); glDisable(GL_LIGHT6); glDisable(GL_LIGHT7);

    // Prepare iterators
    World::Vec_Object::iterator
        MD_curr,
        MD_first = world.objects.begin(),
        MD_last  = world.objects.end();
    Vec_xLight::iterator
        LT_curr,
        LT_first = world.lights.begin(),
        LT_last  = world.lights.end();
    Math::Cameras::CameraSet::Vec_Camera::iterator
        CAM_curr = cameraSet.L_cameras.begin(),
        CAM_last = cameraSet.L_cameras.end();

    //////////////////// WORLD - BEGIN

    if (GLExtensions::Exists_ARB_Multisample)
        glDisable(GL_MULTISAMPLE_ARB);

    glClearColor( world.skyColor.r, world.skyColor.g, world.skyColor.b, world.skyColor.a );
    glDepthMask(1);
    glColorMask(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glShadeModel(GL_SMOOTH);

    for (; CAM_curr != CAM_last; ++CAM_curr)
    {
        Math::Cameras::Camera &camera = **CAM_curr;

        ViewportSet_GL(camera);

        glPushAttrib(GL_ALL_ATTRIB_BITS);

        /////// Render the SKY
        GLShader::Suspend();
        GLShader::SetLightType(xLight_NONE);
        GLShader::EnableTexturing(xState_Enable);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_BLEND);
        if (world.skyBox)
            renderModel.RenderModel(*world.skyBox->ModelGr->xModel, world.skyBox->ModelGr->instance, false, camera.FOV);

        if (GLExtensions::Exists_ARB_Multisample && Config::MultisamplingLevel > 0)
            glEnable(GL_MULTISAMPLE_ARB);

        ////// RENDER Z-ONLY PASS
        glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
        glEnable   (GL_DEPTH_TEST);
        glDepthMask(1);
        glDepthFunc(GL_LESS);
        glEnable   (GL_CULL_FACE);
        glCullFace (GL_BACK);
        glColorMask(0,0,0,0);
        GLShader::SetLightType(xLight_NONE);
        GLShader::EnableTexturing(xState_Off);
        for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            renderModel.RenderDepth( *obj.ModelGr->xModel, obj.ModelGr->instance, false, camera.FOV );
        }

        ////// RENDER GLOBAL AMBIENT PASS
        glDepthMask(0);
        glDepthFunc(GL_LEQUAL);
        glColorMask(1,1,1,1);
        GLShader::SetLightType(xLight_GLOBAL, true, false, false);
        glDisable(GL_LIGHT0);
        for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            renderModel.RenderAmbient( *obj.ModelGr->xModel, obj.ModelGr->instance, world.lights, false, camera.FOV );
        }

        ////// RENDER SHADOWS AND LIGHTS
        for (LT_curr = LT_first ; LT_curr != LT_last ; ++LT_curr)
        {
            if (LT_curr->turned_on && LT_curr->isVisible(camera.FOV))
            {
                if (LT_curr->type != xLight_INFINITE && LT_curr->radius > 0.f)
                {
                    //int x, y, width, height;
                    // set scissor region optimization
                    //getScreenBoundingRectangle(vector4to3(curLight.m_position),
                    //    curLight.m_radius, camera, view,
                    //    vars.m_winWidth, vars.m_winHeight,
                    //    x, y, width, height);

                    //glEnable(GL_SCISSOR_TEST);
                    //glScissor(x, y, width, height);
                }

                ////// SHADOW DETERMINATION PASS
                if (Config::EnableShadows)
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glStencilMask(0xff);
                    glClear(GL_STENCIL_BUFFER_BIT);
                    glEnable(GL_STENCIL_TEST);          // write to stencil buffer
                    if (GLExtensions::Exists_EXT_StencilTwoSide)
                    {
                        glDisable(GL_CULL_FACE);
                        glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
                        glActiveStencilFaceEXT(GL_BACK);
                        glStencilMask(0xff);
                        glStencilFunc(GL_ALWAYS, 0, 0xff);
                        glActiveStencilFaceEXT(GL_FRONT);
                    }
                    else
                        glEnable(GL_CULL_FACE);
                    glStencilMask(0xff);                // allow writing to the first byte of buffer
                    glStencilFunc(GL_ALWAYS, 0, 0xff);  // always pass stencil test
                    glDepthFunc(GL_LESS);
                    glColorMask(0, 0, 0, 0);            // do not write to frame buffer
                    GLShader::EnableTexturing(xState_Disable);
                    GLShader::SetLightType(xLight_NONE);
                    GLShader::Suspend();
                    for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
                    {
                        RigidObj &obj = *(RigidObj*)*MD_curr;
                        if (obj.FL_shadowcaster)
                            renderModel.RenderShadowVolume( *obj.ModelGr->xModel, obj.ModelGr->instance, *LT_curr, camera.FOV );
                    }

                    // Clean-up
                    glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
                    glStencilMask(0);                 // do not write to stencil buffer
                    glStencilFunc(GL_EQUAL, 0, 0xff); // set stencil test function
                }

                ////// ILLUMINATION PASS
                glDepthFunc(GL_LEQUAL);
                glEnable(GL_CULL_FACE);     // enable face culling
                glCullFace(GL_BACK);
                glColorMask(1,1,1,1);
                glEnable(GL_BLEND);                 // add light contribution to frame buffer
                glBlendFunc(GL_ONE, GL_ONE);
                GLShader::EnableTexturing(xState_Enable);
                SetLight(*LT_curr, false, true, true);
                glEnable(GL_LIGHT0);
                for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
                {
                    RigidObj &obj = *(RigidObj*)*MD_curr;
                    renderModel.RenderDiffuse( *obj.ModelGr->xModel, obj.ModelGr->instance, *LT_curr, false, camera.FOV );
                }

                ////// DISPLAY SHADOW VOLUMES PASS
                if (Config::DisplayShadowVolumes)
                {
                    glPushAttrib(GL_ALL_ATTRIB_BITS);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glDisable(GL_STENCIL_TEST);
                    glDepthMask(1);
                    glColorMask(1, 1, 1, 1);
                    glEnable(GL_BLEND);
                    GLShader::EnableTexturing(xState_Disable);
                    GLShader::SetLightType(xLight_NONE);
                    GLShader::Suspend();
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                    for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
                    {
                        RigidObj &obj = *(RigidObj*)*MD_curr;
                        if (obj.FL_shadowcaster)
                            renderModel.RenderShadowVolume( *obj.ModelGr->xModel, obj.ModelGr->instance, *LT_curr, camera.FOV );
                    }
                    glPopAttrib();
                }
            }
            LT_curr->modified = false;
        }

        ////// RENDER TRANSPARENT PASS
        glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
        glDepthMask(0);
        glDepthFunc(GL_LESS);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        GLShader::EnableTexturing(xState_Enable);
        GLShader::SetLightType(xLight_GLOBAL, true, false, false); // 3 * true
        glDisable(GL_LIGHT0);
        for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            renderModel.RenderAmbient( *obj.ModelGr->xModel, obj.ModelGr->instance, world.lights, true, camera.FOV );
        }
        GLShader::Suspend();

        ////// RENDER SKELETONS
        if (Config::DisplaySkeleton)
        {
            GLShader::EnableTexturing(xState_Disable);
            GLShader::SetLightType(xLight_NONE);
            GLShader::Start();
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                renderModel.RenderSkeleton( *obj.ModelGr->xModel, obj.ModelGr->instance, xWORD_MAX );
            }
        }

        ////// RENDER BVHs
        if (Config::DisplayBVH)
        {
            GLShader::EnableTexturing(xState_Disable);
            GLShader::SetLightType(xLight_NONE);
            GLShader::Start();
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glColor3f(1.f, 1.f, 1.f);
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                renderModel.RenderBVH( obj.BVHierarchy, obj.MX_LocalToWorld_Get() );
            }
        }

        glPopAttrib();
    }

    GLShader::Suspend();

    if (GLExtensions::Exists_ARB_Multisample)
        glDisable(GL_MULTISAMPLE_ARB);
}

void WorldRenderGL :: RenderWorld(World &world, xLight &light, xColor sky, Math::Cameras::CameraSet &cameraSet)
{
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // GL_FALSE = infinite viewpoint, GL_TRUE = locale viewpoint
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);    // GL_TRUE=two, GL_FALSE=one
    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_POLYGON_SMOOTH);                       // produces errors on many cards... use FSAA!
    glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2); glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4); glDisable(GL_LIGHT5); glDisable(GL_LIGHT6); glDisable(GL_LIGHT7);

    // Prepare iterators
    World::Vec_Object::iterator
        MD_curr,
        MD_first = world.objects.begin(),
        MD_last  = world.objects.end();
    Math::Cameras::CameraSet::Vec_Camera::iterator
        CAM_curr = cameraSet.L_cameras.begin(),
        CAM_last = cameraSet.L_cameras.end();

    //////////////////// WORLD - BEGIN

    if (GLExtensions::Exists_ARB_Multisample)
        glDisable(GL_MULTISAMPLE_ARB);

    glClearColor( sky.r, sky.g, sky.b, sky.a );
    glDepthMask(1);
    glColorMask(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glShadeModel(GL_SMOOTH);

    for (; CAM_curr != CAM_last; ++CAM_curr)
    {
        Math::Cameras::Camera &camera = **CAM_curr;

        ViewportSet_GL(camera);

        glPushAttrib(GL_ALL_ATTRIB_BITS);

        GLShader::EnableTexturing(xState_Enable);
        if (GLExtensions::Exists_ARB_Multisample && Config::MultisamplingLevel > 0)
            glEnable(GL_MULTISAMPLE_ARB);

        ////// RENDER GLOBAL AMBIENT PASS
        glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
        glEnable   (GL_DEPTH_TEST);
        glDepthMask(1);
        glDepthFunc(GL_LESS);
        glEnable   (GL_CULL_FACE);
        glCullFace (GL_BACK);
        glColorMask(1,1,1,1);
        GLShader::Suspend();
        GLShader::SetLightType(xLight_GLOBAL, true, false, false);
        glDisable(GL_BLEND);
        glDisable(GL_LIGHT0);
        for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            renderModel.RenderAmbient( *obj.ModelGr->xModel, obj.ModelGr->instance, light, false, camera.FOV );
        }

        ////// RENDER SHADOWS AND LIGHTS
        glDepthMask(0);

        ////// SHADOW DETERMINATION PASS
        if (Config::EnableShadows)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glStencilMask(0xff);
            glClear(GL_STENCIL_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);          // write to stencil buffer
            if (GLExtensions::Exists_EXT_StencilTwoSide)
            {
                glDisable(GL_CULL_FACE);
                glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
                glActiveStencilFaceEXT(GL_BACK);
                glStencilMask(0xff);
                glStencilFunc(GL_ALWAYS, 0, 0xff);
                glActiveStencilFaceEXT(GL_FRONT);
            }
            else
                glEnable(GL_CULL_FACE);
            glStencilMask(0xff);                // allow writing to the first byte of buffer
            glStencilFunc(GL_ALWAYS, 0, 0xff);  // always pass stencil test
            glDepthFunc(GL_LESS);
            glColorMask(0, 0, 0, 0);            // do not write to frame buffer
            GLShader::EnableTexturing(xState_Disable);
            GLShader::SetLightType(xLight_NONE);
            GLShader::Suspend();
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                if (obj.FL_shadowcaster)
                    renderModel.RenderShadowVolume( *obj.ModelGr->xModel, obj.ModelGr->instance, light, camera.FOV );
            }
            // Clean-up
            glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
            glStencilMask(0);                 // do not write to stencil buffer
            glStencilFunc(GL_EQUAL, 0, 0xff); // set stencil test function
        }

        ////// ILLUMINATION PASS
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_CULL_FACE);     // enable face culling
        glCullFace(GL_BACK);
        glColorMask(1,1,1,1);
        glEnable(GL_BLEND);                 // add light contribution to frame buffer
        glBlendFunc(GL_ONE, GL_ONE);
        GLShader::EnableTexturing(xState_Enable);
        SetLight(light, false, true, true);
        glEnable(GL_LIGHT0);
        for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            renderModel.RenderDiffuse( *obj.ModelGr->xModel, obj.ModelGr->instance, light, false, camera.FOV );
        }

        ////// DISPLAY SHADOW VOLUMES PASS
        if (Config::DisplayShadowVolumes)
        {
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_STENCIL_TEST);
            glDepthMask(1);
            glColorMask(1, 1, 1, 1);
            glEnable(GL_BLEND);
            GLShader::EnableTexturing(xState_Disable);
            GLShader::SetLightType(xLight_NONE);
            GLShader::Suspend();
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                if (obj.FL_shadowcaster)
                    renderModel.RenderShadowVolume( *obj.ModelGr->xModel, obj.ModelGr->instance, light, camera.FOV );
            }
            glPopAttrib();
        }

        light.modified = false;

        ////// RENDER TRANSPARENT PASS
        glDepthMask(0);
        glDepthFunc(GL_LESS);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        GLShader::EnableTexturing(xState_Enable);
        GLShader::SetLightType(xLight_GLOBAL, true, false, false); // 3 * true
        glDisable(GL_LIGHT0);
        for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            renderModel.RenderAmbient( *obj.ModelGr->xModel, obj.ModelGr->instance, light, true, camera.FOV );
        }
        GLShader::Suspend();

        ////// RENDER SKELETONS
        if (Config::DisplaySkeleton)
        {
            GLShader::EnableTexturing(xState_Disable);
            GLShader::SetLightType(xLight_NONE);
            GLShader::Start();
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                renderModel.RenderSkeleton( *obj.ModelGr->xModel, obj.ModelGr->instance, xWORD_MAX );
            }
        }

        ////// RENDER BVHs
        if (Config::DisplayBVH)
        {
            GLShader::EnableTexturing(xState_Disable);
            GLShader::SetLightType(xLight_NONE);
            GLShader::Start();
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glColor3f(1.f, 1.f, 1.f);
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                renderModel.RenderBVH( obj.BVHierarchy, obj.MX_LocalToWorld_Get() );
            }
        }

        glPopAttrib();
    }

    GLShader::Suspend();

    if (GLExtensions::Exists_ARB_Multisample)
        glDisable(GL_MULTISAMPLE_ARB);
}

void WorldRenderGL :: RenderWorldNoLights(World &world, xColor sky, Math::Cameras::CameraSet &cameraSet)
{
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // GL_FALSE = infinite viewpoint, GL_TRUE = locale viewpoint
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);    // GL_TRUE=two, GL_FALSE=one
    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_POLYGON_SMOOTH);                       // produces errors on many cards... use FSAA!
    glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2); glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4); glDisable(GL_LIGHT5); glDisable(GL_LIGHT6); glDisable(GL_LIGHT7);

    // Prepare iterators
    World::Vec_Object::iterator
        MD_curr,
        MD_first = world.objects.begin(),
        MD_last  = world.objects.end();
    Math::Cameras::CameraSet::Vec_Camera::iterator
        CAM_curr = cameraSet.L_cameras.begin(),
        CAM_last = cameraSet.L_cameras.end();

    //////////////////// WORLD - BEGIN

    if (GLExtensions::Exists_ARB_Multisample)
        if (Config::MultisamplingLevel > 0)
            glEnable(GL_MULTISAMPLE_ARB);
        else
            glDisable(GL_MULTISAMPLE_ARB);

    glClearColor( sky.r, sky.g, sky.b, sky.a );
    glDepthMask(1);
    glColorMask(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glShadeModel(GL_SMOOTH);

    for (; CAM_curr != CAM_last; ++CAM_curr)
    {
        Math::Cameras::Camera &camera = **CAM_curr;

        ViewportSet_GL(camera);

        glPushAttrib(GL_ALL_ATTRIB_BITS);

        ////// RENDER OPAQUE PASS
        glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
        glEnable   (GL_DEPTH_TEST);
        glDepthMask(1);
        glDepthFunc(GL_LESS);
        glEnable   (GL_CULL_FACE);
        glCullFace (GL_BACK);
        glColorMask(1, 1, 1, 1);
        GLShader::Suspend();
        GLShader::EnableTexturing(xState_Enable);
        GLShader::SetLightType(xLight_NONE);
        for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            renderModel.RenderAmbient( *obj.ModelGr->xModel, obj.ModelGr->instance, world.lights, false, camera.FOV );
        }

        ////// RENDER TRANSPARENT PASS
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            renderModel.RenderAmbient( *obj.ModelGr->xModel, obj.ModelGr->instance, world.lights, true, camera.FOV );
        }
        GLShader::Suspend();

        ////// RENDER SKELETONS
        if (Config::DisplaySkeleton)
        {
            GLShader::EnableTexturing(xState_Disable);
            GLShader::SetLightType(xLight_NONE);
            GLShader::Start();
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                renderModel.RenderSkeleton( *obj.ModelGr->xModel, obj.ModelGr->instance, xWORD_MAX );
            }
        }

        ////// RENDER BVHs
        if (Config::DisplayBVH)
        {
            GLShader::EnableTexturing(xState_Disable);
            GLShader::SetLightType(xLight_NONE);
            GLShader::Start();
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glColor3f(1.f, 1.f, 1.f);
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                renderModel.RenderBVH( obj.BVHierarchy, obj.MX_LocalToWorld_Get() );
            }
        }

        glPopAttrib();
    }

    GLShader::Suspend();

    if (GLExtensions::Exists_ARB_Multisample)
        glDisable(GL_MULTISAMPLE_ARB);
}
