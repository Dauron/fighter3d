#include "WorldRenderGL.h"

#include "Utils.h"
#include "Shader.h"
#include "Extensions/EXT_stencil_wrap.h"
#include "Extensions/EXT_stencil_two_side.h"
#include "Extensions/ARB_multisample.h"

#include "../../Utils/Profiler.h"

using namespace Graphics::OGL;

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

    // rozpraszanie siê œwiat³a
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

    Shader::SetLightType(light.type, t_Ambient, t_Diffuse, t_Specular);
}

void WorldRenderGL :: RenderWorld(World &world, Math::Cameras::CameraSet &cameraSet)
{
    Profile("Render world");

    glHint(GL_POINT_SMOOTH_HINT,           GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT,            GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,         GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nice perspective calculations

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // GL_FALSE = infinite viewpoint, GL_TRUE = locale viewpoint
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);    // GL_TRUE=two, GL_FALSE=one
    glDisable   (GL_LINE_SMOOTH);
    glDisable   (GL_POLYGON_SMOOTH);                     // produces errors on many cards... use FSAA!
    glEnable    (GL_POINT_SMOOTH);
    glShadeModel(GL_SMOOTH); // GL_SMOOTH - enable smooth shading, GL_FLAT - no gradient on faces
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

    for (; CAM_curr != CAM_last; ++CAM_curr)
    {
        Profile("Camera view");

        Math::Cameras::Camera &camera = **CAM_curr;

        ViewportSet_GL(camera);

        glPushAttrib(GL_ALL_ATTRIB_BITS);

        /////// Render the SKY
        Shader::Suspend();
        Shader::SetLightType(xLight_NONE);
        Shader::EnableTexturing(xState_Enable);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glFrontFace(GL_CCW);     // Front faces are drawn in counter-clockwise direction
        glEnable   (GL_CULL_FACE);
        glCullFace (GL_BACK);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_BLEND);
        if (world.skyBox)
            renderModel.RenderModel(*world.skyBox->ModelGr->xModelP, world.skyBox->ModelGr->instance, false, camera.FOV);

        if (GLExtensions::Exists_ARB_Multisample && Config::MultisamplingLevel > 0)
            glEnable(GL_MULTISAMPLE_ARB);

        ////// RENDER Z-ONLY PASS
        {
            Profile("Z-only pass");

            glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
            glEnable   (GL_DEPTH_TEST);
            glDepthMask(1);
            glDepthFunc(GL_LESS);
            glColorMask(0,0,0,0);
            Shader::SetLightType(xLight_NONE);
            Shader::EnableTexturing(xState_Off);
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                renderModel.RenderDepth( *obj.ModelGr->xModelP, obj.ModelGr->instance, false, camera.FOV );
            }
        }

        ////// RENDER GLOBAL AMBIENT PASS
        {
            Profile("Ambient pass");

            glDepthMask(0);
            glDepthFunc(GL_LEQUAL);
            glColorMask(1,1,1,1);
            Shader::SetLightType(xLight_GLOBAL, true, false, false);
            glDisable(GL_LIGHT0);
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                renderModel.RenderAmbient( *obj.ModelGr->xModelP, obj.ModelGr->instance, world.lights, false, camera.FOV );
            }
        }

        ////// RENDER SHADOWS AND LIGHTS
        for (LT_curr = LT_first ; LT_curr != LT_last ; ++LT_curr)
        {
            if (LT_curr->turned_on && LT_curr->isVisible(camera.FOV))
            {
                Profile("Light pass");

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
                    Profile("Cast shadow volumes pass");

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
                    Shader::EnableTexturing(xState_Disable);
                    Shader::SetLightType(xLight_NONE);
                    Shader::Suspend();
                    for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
                    {
                        RigidObj &obj = *(RigidObj*)*MD_curr;
                        if (obj.FL_shadowcaster)
                            renderModel.RenderShadowVolume( *obj.ModelGr->xModelP, obj.ModelGr->instance, *LT_curr, camera.FOV );
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
                Shader::EnableTexturing(xState_Enable);
                SetLight(*LT_curr, false, true, true);
                glEnable(GL_LIGHT0);
                for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
                {
                    RigidObj &obj = *(RigidObj*)*MD_curr;
                    renderModel.RenderDiffuse( *obj.ModelGr->xModelP, obj.ModelGr->instance, *LT_curr, false, camera.FOV );
                }

                ////// DISPLAY SHADOW VOLUMES PASS
                if (Config::DisplayShadowVolumes)
                {
                    Profile("Display shadow volumes pass");

                    glPushAttrib(GL_ALL_ATTRIB_BITS);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glDisable(GL_STENCIL_TEST);
                    glDepthMask(1);
                    glColorMask(1, 1, 1, 1);
                    glEnable(GL_BLEND);
                    Shader::EnableTexturing(xState_Disable);
                    Shader::SetLightType(xLight_NONE);
                    Shader::Suspend();
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                    for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
                    {
                        RigidObj &obj = *(RigidObj*)*MD_curr;
                        if (obj.FL_shadowcaster)
                            renderModel.RenderShadowVolume( *obj.ModelGr->xModelP, obj.ModelGr->instance, *LT_curr, camera.FOV );
                    }
                    glPopAttrib();
                }
            }
            LT_curr->modified = false;
        }

        ////// RENDER TRANSPARENT PASS
        {
            Profile("Transparent pass");

            glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
            glDepthMask(0);
            glDepthFunc(GL_LESS);
            glDisable(GL_STENCIL_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            Shader::EnableTexturing(xState_Enable);
            Shader::SetLightType(xLight_GLOBAL, true, false, false); // 3 * true
            glDisable(GL_LIGHT0);
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                renderModel.RenderAmbient( *obj.ModelGr->xModelP, obj.ModelGr->instance, world.lights, true, camera.FOV );
            }
        }
        Shader::Suspend();

        ////// RENDER SKELETONS
        if (Config::DisplaySkeleton)
        {
            Profile("Skeleton pass");

            Shader::EnableTexturing(xState_Disable);
            Shader::SetLightType(xLight_NONE);
            Shader::Start();
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                renderModel.RenderSkeleton( *obj.ModelGr->xModelP, obj.ModelGr->instance, xWORD_MAX );
            }
        }

        ////// RENDER BVHs
        if (Config::DisplayBVH)
        {
            Profile("BVH pass");

            Shader::EnableTexturing(xState_Disable);
            Shader::SetLightType(xLight_NONE);
            Shader::Start();
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

    Shader::Suspend();

    if (GLExtensions::Exists_ARB_Multisample)
        glDisable(GL_MULTISAMPLE_ARB);
}

void WorldRenderGL :: RenderWorld(World &world, xLight &light, xColor sky, Math::Cameras::CameraSet &cameraSet)
{
    glHint(GL_POINT_SMOOTH_HINT,           GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT,            GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,         GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nice perspective calculations

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // GL_FALSE = infinite viewpoint, GL_TRUE = locale viewpoint
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);    // GL_TRUE=two, GL_FALSE=one
    glDisable   (GL_LINE_SMOOTH);
    glDisable   (GL_POLYGON_SMOOTH);                     // produces errors on many cards... use FSAA!
    glEnable    (GL_POINT_SMOOTH);
    glShadeModel(GL_SMOOTH); // GL_SMOOTH - enable smooth shading, GL_FLAT - no gradient on faces
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

    for (; CAM_curr != CAM_last; ++CAM_curr)
    {
        Math::Cameras::Camera &camera = **CAM_curr;

        ViewportSet_GL(camera);

        glPushAttrib(GL_ALL_ATTRIB_BITS);

        Shader::EnableTexturing(xState_Enable);
        if (GLExtensions::Exists_ARB_Multisample && Config::MultisamplingLevel > 0)
            glEnable(GL_MULTISAMPLE_ARB);

        ////// RENDER GLOBAL AMBIENT PASS
        glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
        glEnable   (GL_DEPTH_TEST);
        glDepthMask(1);
        glDepthFunc(GL_LESS);
        glFrontFace(GL_CCW);     // Front faces are drawn in counter-clockwise direction
        glEnable   (GL_CULL_FACE);
        glCullFace (GL_BACK);
        glColorMask(1,1,1,1);
        Shader::Suspend();
        Shader::SetLightType(xLight_GLOBAL, true, false, false);
        glDisable(GL_BLEND);
        glDisable(GL_LIGHT0);
        for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            renderModel.RenderAmbient( *obj.ModelGr->xModelP, obj.ModelGr->instance, light, false, camera.FOV );
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
            Shader::EnableTexturing(xState_Disable);
            Shader::SetLightType(xLight_NONE);
            Shader::Suspend();
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                if (obj.FL_shadowcaster)
                    renderModel.RenderShadowVolume( *obj.ModelGr->xModelP, obj.ModelGr->instance, light, camera.FOV );
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
        Shader::EnableTexturing(xState_Enable);
        SetLight(light, false, true, true);
        glEnable(GL_LIGHT0);
        for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            renderModel.RenderDiffuse( *obj.ModelGr->xModelP, obj.ModelGr->instance, light, false, camera.FOV );
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
            Shader::EnableTexturing(xState_Disable);
            Shader::SetLightType(xLight_NONE);
            Shader::Suspend();
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                if (obj.FL_shadowcaster)
                    renderModel.RenderShadowVolume( *obj.ModelGr->xModelP, obj.ModelGr->instance, light, camera.FOV );
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
        Shader::EnableTexturing(xState_Enable);
        Shader::SetLightType(xLight_GLOBAL, true, false, false); // 3 * true
        glDisable(GL_LIGHT0);
        for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            renderModel.RenderAmbient( *obj.ModelGr->xModelP, obj.ModelGr->instance, light, true, camera.FOV );
        }
        Shader::Suspend();

        ////// RENDER SKELETONS
        if (Config::DisplaySkeleton)
        {
            Shader::EnableTexturing(xState_Disable);
            Shader::SetLightType(xLight_NONE);
            Shader::Start();
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                renderModel.RenderSkeleton( *obj.ModelGr->xModelP, obj.ModelGr->instance, xWORD_MAX );
            }
        }

        ////// RENDER BVHs
        if (Config::DisplayBVH)
        {
            Shader::EnableTexturing(xState_Disable);
            Shader::SetLightType(xLight_NONE);
            Shader::Start();
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

    Shader::Suspend();

    if (GLExtensions::Exists_ARB_Multisample)
        glDisable(GL_MULTISAMPLE_ARB);
}

void WorldRenderGL :: RenderWorldNoLights(World &world, xColor sky, Math::Cameras::CameraSet &cameraSet)
{
    glHint(GL_POINT_SMOOTH_HINT,           GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT,            GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,         GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nice perspective calculations

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // GL_FALSE = infinite viewpoint, GL_TRUE = locale viewpoint
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);    // GL_TRUE=two, GL_FALSE=one
    glDisable   (GL_LINE_SMOOTH);
    glDisable   (GL_POLYGON_SMOOTH);                     // produces errors on many cards... use FSAA!
    glEnable    (GL_POINT_SMOOTH);
    glShadeModel(GL_SMOOTH); // GL_SMOOTH - enable smooth shading, GL_FLAT - no gradient on faces
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
        glFrontFace(GL_CCW);     // Front faces are drawn in counter-clockwise direction
        glEnable   (GL_CULL_FACE);
        glCullFace (GL_BACK);
        glColorMask(1, 1, 1, 1);
        Shader::Suspend();
        Shader::EnableTexturing(xState_Enable);
        Shader::SetLightType(xLight_NONE);
        for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            renderModel.RenderAmbient( *obj.ModelGr->xModelP, obj.ModelGr->instance, world.lights, false, camera.FOV );
        }

        ////// RENDER TRANSPARENT PASS
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
        {
            RigidObj &obj = *(RigidObj*)*MD_curr;
            renderModel.RenderAmbient( *obj.ModelGr->xModelP, obj.ModelGr->instance, world.lights, true, camera.FOV );
        }
        Shader::Suspend();

        ////// RENDER SKELETONS
        if (Config::DisplaySkeleton)
        {
            Shader::EnableTexturing(xState_Disable);
            Shader::SetLightType(xLight_NONE);
            Shader::Start();
            for ( MD_curr = MD_first ; MD_curr != MD_last ; ++MD_curr )
            {
                RigidObj &obj = *(RigidObj*)*MD_curr;
                renderModel.RenderSkeleton( *obj.ModelGr->xModelP, obj.ModelGr->instance, xWORD_MAX );
            }
        }

        ////// RENDER BVHs
        if (Config::DisplayBVH)
        {
            Shader::EnableTexturing(xState_Disable);
            Shader::SetLightType(xLight_NONE);
            Shader::Start();
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

    Shader::Suspend();

    if (GLExtensions::Exists_ARB_Multisample)
        glDisable(GL_MULTISAMPLE_ARB);
}
