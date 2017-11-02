#ifndef __incl_Graphics_OGL_DeferredFBO_h
#define __incl_Graphics_OGL_DeferredFBO_h

#include "ogl.h"
#include "Utils.h"
#include "../../Math/Cameras/Camera.h"

namespace Graphics { namespace OGL {

    struct DeferredFBO
    {
    private:
        int    Width, Height;

        GLuint FBO_deferred;
        GLuint FBO_lightAccum;
        GLuint FBO_filter1;
        GLuint FBO_filter2;

        GLuint FBO_current;
        float  sampleWidth, sampleHeight;
        float  texScale;

        GLuint TEX_depth_stencil;

        GLuint TEX_deferred_position;
        GLuint TEX_deferred_normal;
        GLuint TEX_deferred_material;

        GLuint TEX_light_diffuse;
        GLuint TEX_light_specular;

        GLuint TEX_filter1;
        GLuint TEX_filter2;

    public:
        void Clear()  { memset(this, 0, sizeof(DeferredFBO)); }

        DeferredFBO() { Clear(); }

        bool Create ( int width, int height );
        bool Resize ( int width, int height );
        void Destroy();

        void Stage1_MRT(xColor color);
        void Stage2_PreLights();
        void Stage2_InfiniteLight(xLight &light);
        void Stage2_PointLight(xLight &light, Math::Cameras::Camera &camera);
        void Stage2_PostLights();
        void Stage3_Join();

        void Stage4_PreFilters();
        void Stage4_ConvolutionFilter(xFLOAT Mask[9]);
        void Stage4_Resize(float scale);
        void Stage4_Tint(xColor color, float treshold);

        void Stage8_Fin();
        void Stage9_Test();

        bool IsValid () { return FBO_deferred; }
    };

} } // namespace Graphics::OGL

#endif
