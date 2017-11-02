#ifdef WIN32

#include "PBuffer.h"
#include "../../Utils/Debug.h"
#include <cassert>

using namespace Graphics::OGL;

bool PBuffer:: Create(int width, int height,
                      const int * attribIList, const float * attribFList, const int * flags)
{
    assert(!hBuffer && "PBuffer is already created");

    //Check for pbuffer support
    if (!WGLEW_ARB_pixel_format ||
        !WGLEW_ARB_pbuffer)
    {
        LOG(2, "PBuffer::Create : Extension required for PBuffer unsupported");
        return false;
    }

    //Get the current device context
    HDC hCurrentDC = wglGetCurrentDC();
    if(!hCurrentDC)
    {
        LOG(2, "PBuffer::Create : Unable to get current Device Context");
        return false;
    }

    //choose pixel format
    GLint pixelFormat;
    unsigned int numFormats;

    if(!wglChoosePixelFormatARB(hCurrentDC, attribIList, attribFList, 1,
                                &pixelFormat, &numFormats))
    {
        LOG(2, "PBuffer::Create : Unable to find a pixel format for the PBuffer");
        return false;
    }

    //Create the pbuffer
    hBuffer = wglCreatePbufferARB(hCurrentDC, pixelFormat, width, height, flags);
    if(!hBuffer)
    {
        LOG(2, "PBuffer::Create : Unable to create PBuffer");
        return false;
    }

    //Get the pbuffer's device context
    hDC = wglGetPbufferDCARB(hBuffer);
    if(!hDC)
    {
        LOG(2, "PBuffer::Create : Unable to get PBuffer's device context");
        return false;
    }

    //Create a rendering context for the pbuffer
    hRC = wglCreateContext(hDC);
    if(!hRC)
    {
        LOG(2, "PBuffer::Create : Unable to create PBuffer's rendering context");
        return false;
    }

    //Set and output the actual pBuffer dimensions
    Width  = width;
    Height = height;
    //wglQueryPbufferARB(hBuffer, WGL_PBUFFER_WIDTH_ARB,  &width);
    //wglQueryPbufferARB(hBuffer, WGL_PBUFFER_HEIGHT_ARB, &height);
    LOG(2, "PBuffer::Create : Pbuffer Created: (%d x %d)", width, height);

    return true;
}

void PBuffer:: Destroy(void)
{
    if(hRC)
    {
        if(!wglDeleteContext(hRC))    //try to delete RC
            LOG(2, "PBuffer::Destroy : Release of Pbuffer Rendering Context Failed");
        hRC = 0;
    }

    if(hDC)
    {
        if (!wglReleasePbufferDCARB(hBuffer, hDC)) //Are we able to release DC?
            LOG(2, "PBuffer::Destroy : Release of Pbuffer Device Context Failed");
        hDC = 0;
    }

    if (hBuffer)
    {
        if(!wglDestroyPbufferARB(hBuffer))
            LOG(2, "PBuffer::Destroy : Unable to destroy pbuffer");
        hBuffer = 0;
    }
}


bool PBuffer:: MakeCurrent()
{
    assert(hBuffer && "PBuffer is not created");

    if(!wglMakeCurrent(hDC, hRC))
    {
        LOG(2, "PBuffer::MakeCurrent : Unable to change current context");
        return false;
    }

    return true;
}
#endif //WIN32

