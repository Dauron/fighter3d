#ifndef __incl_Graphics_OGL_PBuffer_h
#define __incl_Graphics_OGL_PBuffer_h

#include "ogl.h"

#ifdef WIN32

namespace Graphics { namespace OGL {

    struct PBuffer
    {
    private:
        HGLRC hRC;                      //rendering context
        HDC   hDC;                      //device context
        HPBUFFERARB hBuffer;            //buffer handle

        int Width, Height;              //pBuffer size

    public:
        void Clear()
        {
            hRC     = 0;
            hDC     = 0;
            hBuffer = 0;
        }

        PBuffer() { Clear(); }

        bool Create ( int width, int height,
                      const int * attribIList, const float * attribFList, const int * flags);
        void Destroy();

        bool IsValid ()
        {
            if (!hBuffer) return false;

            int isLost;
            wglQueryPbufferARB(hBuffer, WGL_PBUFFER_LOST_ARB, &isLost);
            return isLost != TRUE;
        }

        bool MakeCurrent();
    };

} } // namespace Graphics::OGL

#endif // WIN32

#endif
