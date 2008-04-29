#ifndef __GraphicsModes_h
#define __GraphicsModes_h

#ifdef WIN32

#include "../AppFramework/System.h"

class GraphicsModes
{
    HDC hdc;

public:
    GraphicsModes(HDC hdc)
    {
        this->hdc = hdc;
    }

    int CountModes()
    {
        return DescribePixelFormat(hdc, 1, 0, NULL);
    }

    PIXELFORMATDESCRIPTOR* ListModes()
    {
        int cnt = CountModes();

        PIXELFORMATDESCRIPTOR *pfds = new PIXELFORMATDESCRIPTOR[cnt];

        for (int iPixelFormat = 0; iPixelFormat < cnt; ++iPixelFormat)
        {
            // obtain detailed information about
            // the device context's first pixel format
            DescribePixelFormat(hdc, iPixelFormat+1,
                sizeof(PIXELFORMATDESCRIPTOR), pfds+iPixelFormat);
        }

        return pfds;
    }

    std::string ModeToString(PIXELFORMATDESCRIPTOR pfd)
    {
        std::string result;

        result += "    iVersion: " + itos(pfd.nVersion) + '\n';

        result += "    dwFlags:\n";
        if (pfd.dwFlags & PFD_DOUBLEBUFFER)
            result += "        PFD_DOUBLEBUFFER\n";
        if (pfd.dwFlags & PFD_STEREO)
            result += "        PFD_STEREO\n";
        if (pfd.dwFlags & PFD_DRAW_TO_WINDOW)
            result += "        PFD_DRAW_TO_WINDOW\n";
        if (pfd.dwFlags & PFD_DRAW_TO_BITMAP)
            result += "        PFD_DRAW_TO_BITMAP\n";
        if (pfd.dwFlags & PFD_SUPPORT_GDI)
            result += "        PFD_SUPPORT_GDI\n";
        if (pfd.dwFlags & PFD_SUPPORT_OPENGL)
            result += "        PFD_SUPPORT_OPENGL\n";
        if (pfd.dwFlags & PFD_GENERIC_FORMAT)
            result += "        PFD_GENERIC_FORMAT\n";
        if (pfd.dwFlags & PFD_NEED_PALETTE)
            result += "        PFD_NEED_PALETTE\n";
        if (pfd.dwFlags & PFD_NEED_SYSTEM_PALETTE)
            result += "        PFD_NEED_SYSTEM_PALETTE\n";
        if (pfd.dwFlags & PFD_SWAP_EXCHANGE)
            result += "        PFD_SWAP_EXCHANGE\n";
        if (pfd.dwFlags & PFD_SWAP_COPY)
            result += "        PFD_SWAP_COPY\n";
        if (pfd.dwFlags & PFD_SWAP_LAYER_BUFFERS)
            result += "        PFD_SWAP_LAYER_BUFFERS\n";
        if (pfd.dwFlags & PFD_GENERIC_ACCELERATED)
            result += "        PFD_GENERIC_ACCELERATED\n";
        if (pfd.dwFlags & PFD_SUPPORT_DIRECTDRAW)
            result += "        PFD_SUPPORT_DIRECTDRAW\n";
        if (pfd.dwFlags & PFD_DEPTH_DONTCARE)
            result += "        PFD_DEPTH_DONTCARE\n";
        if (pfd.dwFlags & PFD_DOUBLEBUFFER_DONTCARE)
            result += "        PFD_DOUBLEBUFFER_DONTCARE\n";
        if (pfd.dwFlags & PFD_STEREO_DONTCARE)
            result += "        PFD_STEREO_DONTCARE\n";

        if (pfd.iPixelType == PFD_TYPE_RGBA)
            result += "    iPixelType: PFD_TYPE_RGBA\n";
        else
            result += "    iPixelType: PFD_TYPE_COLORINDEX\n";

        result += "    cColorBits: " + itos(pfd.cColorBits) + '\n';
        result += "    cAlphaBits: " + itos(pfd.cAlphaBits) + '\n';
        result += "    cAccumBits: " + itos(pfd.cAccumBits) + '\n';
        result += "    cDepthBits: " + itos(pfd.cDepthBits) + '\n';
        result += "    cStencilBits: " + itos(pfd.cStencilBits) + '\n';
        result += "    cAuxBuffers: " + itos(pfd.cAuxBuffers) + '\n';

        result += "    cAuxBuffers: " + itos(pfd.cAuxBuffers) + '\n';
        result += "    cAuxBuffers: " + itos(pfd.cAuxBuffers) + '\n';

        result += "    iLayerType: ";
        switch (pfd.iLayerType)
        {
            case PFD_MAIN_PLANE:
                result += "PFD_MAIN_PLANE";
                break;
            case PFD_OVERLAY_PLANE:
                result += "PFD_OVERLAY_PLANE";
                break;
            case PFD_UNDERLAY_PLANE:
                result += "PFD_UNDERLAY_PLANE";
                break;
        }
        result += '\n';

        return result;
    }
};

#endif

#endif

