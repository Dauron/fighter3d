#ifdef WIN32

#include "IWindow.h"
#include "Input/InputMgr.h"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Get GLWindow associated with give hWnd
    IWindow* thisWnd = (IWindow*) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch(uMsg)
    {
        case WM_KEYDOWN:                                          // Is A Key Being Held Down?
        case WM_SYSKEYDOWN:
            if (thisWnd)
            {
                if (wParam == VK_SHIFT)
                {
                    bool right = (lParam & (54 << 16)) == (54 << 16);
                    int vkey = right ? VK_RSHIFT : VK_LSHIFT;
                    g_InputMgr.KeyDown_Set((byte)vkey, true);
                }
                if (wParam == VK_CONTROL || wParam == VK_MENU)    // mark L/R ctrl/alt
                {
                    bool right = (lParam & (1 << 24));
                    int vkey;
                    bool rCtrl = GetKeyState(VK_RCONTROL);
                    bool lCtrl = GetKeyState(VK_LCONTROL);
                    if (wParam == VK_CONTROL)
                    {
                        //if (lCtrl && rCtrl) break;                // fake ctrl when R_Alt is pressed
                        vkey = right ? VK_RCONTROL : VK_LCONTROL;
                    }
                    else
                    if (wParam == VK_MENU)
                        vkey = right ? VK_RMENU : VK_LMENU;
                    g_InputMgr.KeyDown_Set((byte)vkey, true);
                }
                g_InputMgr.KeyDown_Set((byte)wParam, true); // If So, Mark It As true
            }
            break;

        case WM_KEYUP:                                            // Has A Key Been Released?
        case WM_SYSKEYUP:
            if (thisWnd)
            {
                if (wParam == VK_SHIFT)
                {
                    bool right = (lParam & (54 << 16)) == (54 << 16);
                    int vkey = right ? VK_RSHIFT : VK_LSHIFT;
                    g_InputMgr.KeyDown_Set((byte)vkey, false);
                }
                if (wParam == VK_CONTROL || wParam == VK_MENU)    // mark L/R ctrl/alt
                {
                    bool right = (lParam & (1 << 24));
                    int vkey;
                    bool rCtrl = GetKeyState(VK_RCONTROL);
                    bool lCtrl = GetKeyState(VK_LCONTROL);
                    if (wParam == VK_CONTROL)
                    {
                        //if (lCtrl && rCtrl) break;                // fake ctrl when R_Alt is pressed
                        vkey = right ? VK_RCONTROL : VK_LCONTROL;
                    }
                    else
                    if (wParam == VK_MENU)
                        vkey = right ? VK_RMENU : VK_LMENU;
                    g_InputMgr.KeyDown_Set((byte)vkey, false);
                }
                g_InputMgr.KeyDown_Set((byte)wParam, false); // If So, Mark It As false
            }
            break;

        case WM_CHAR:                                             // Translated character
            if (thisWnd)
                g_InputMgr.AppendBuffer((byte)wParam);       // Add it to buffer
            break;

        case WM_LBUTTONDOWN:
            g_InputMgr.KeyDown_Set(VK_LBUTTON, true);
            break;
        case WM_MBUTTONDOWN:
            g_InputMgr.KeyDown_Set(VK_MBUTTON, true);
            break;
        case WM_RBUTTONDOWN:
            g_InputMgr.KeyDown_Set(VK_RBUTTON, true);
            break;
        case WM_LBUTTONUP:
            g_InputMgr.KeyDown_Set(VK_LBUTTON, false);
            break;
        case WM_MBUTTONUP:
            g_InputMgr.KeyDown_Set(VK_MBUTTON, false);
            break;
        case WM_RBUTTONUP:
            g_InputMgr.KeyDown_Set(VK_RBUTTON, false);
            break;
        case WM_MOUSEMOVE:
            g_InputMgr.mouseX = LOWORD(lParam);
            g_InputMgr.mouseY = HIWORD(lParam);
            break;
        case WM_MOUSEWHEEL:
            g_InputMgr.mouseWheel += (short) HIWORD(wParam);
            break;

        case WM_SIZE:
            if (thisWnd)
                thisWnd->Resize (LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_ACTIVATE:                            // Watch For Window Activate Message
            if (thisWnd)
                if (!HIWORD(wParam))                 // Check Minimization State
                    thisWnd->Active_Set(TRUE);        // Program Is Active
                else
                    thisWnd->Active_Set(FALSE);       // Program Is No Longer Active
            break;

        case WM_SYSCOMMAND:                          // Intercept System Commands
            switch (wParam)                          // Check System Calls
            {
                case SC_SCREENSAVE:                  // Screensaver Trying To Start?
                case SC_MONITORPOWER:                // Monitor Trying To Enter Powersave?
                return 0;                            // Prevent From Happening
            }
            break;

        case WM_CREATE:
            break;

        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;                                // Skip default action, we will handle it in the main loop

        case WM_DESTROY:
            break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#endif
