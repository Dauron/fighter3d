#ifndef WIN32

#include "IWindow.h"
#include "Input/InputMgr.h"

int TranslateXKeyToVKey(KeySym key);

bool WindowProc(IWindow *thisWnd, XEvent &event)
{
    KeySym key;
    char buff[10];

    switch (event.type)
    {
        case Expose:
            //if (event.xexpose.count != 0)
            //    break;
            //drawGLScene();
            break;
        case ConfigureNotify:
            if (((long)event.xconfigure.width != (long)thisWnd->width) ||
                ((long)event.xconfigure.height != (long)thisWnd->height))
                thisWnd->Resize (event.xconfigure.width, event.xconfigure.height);
            break;

        case KeyPress:
            key = XLookupKeysym(&event.xkey, 0);
            if (key < 256)
            {
                g_InputMgr.SetKeyState((unsigned char)toupper(key), true);
                XLookupString(&event.xkey, buff, 10, &key, NULL);
                g_InputMgr.AppendBuffer((unsigned char)buff[0]);       // Add it to buffer
            }
            else
                g_InputMgr.SetKeyState((unsigned char)TranslateXKeyToVKey(key), true);
            break;
        case KeyRelease:
            key = XLookupKeysym(&event.xkey, 0);
            if (key < 256)
                g_InputMgr.SetKeyState((unsigned char)toupper(key), false);
            else
                g_InputMgr.SetKeyState((unsigned char)TranslateXKeyToVKey(key), false);
            break;

        case ButtonPress:
            if (event.xbutton.button == Button1)
                g_InputMgr.SetKeyState(VK_LBUTTON, true);
            if (event.xbutton.button == Button2)
                g_InputMgr.SetKeyState(VK_RBUTTON, true);
            if (event.xbutton.button == Button3)
                g_InputMgr.SetKeyState(VK_MBUTTON, true);
            if (event.xbutton.button == Button4)
                g_InputMgr.mouseWheel += 120;
            if (event.xbutton.button == Button5)
                g_InputMgr.mouseWheel -= 120;
            break;
        case ButtonRelease:
            if (event.xbutton.button == Button1)
                g_InputMgr.SetKeyState(VK_LBUTTON, false);
            if (event.xbutton.button == Button2)
                g_InputMgr.SetKeyState(VK_RBUTTON, false);
            if (event.xbutton.button == Button3)
                g_InputMgr.SetKeyState(VK_MBUTTON, false);
            break;
        case MotionNotify:
            g_InputMgr.mouseX = event.xmotion.x;
            g_InputMgr.mouseY = event.xmotion.y;
            break;

        case ClientMessage:
            if (*XGetAtomName(thisWnd->hDC, event.xclient.message_type) == *"WM_PROTOCOLS")
            {
                thisWnd->Destroy();
                return false;
            }
            break;

        case FocusIn:
            thisWnd->Active_Set(true);        // Program Is Active
            break;
        case FocusOut:
            thisWnd->Active_Set(false);       // Program Is No Longer Active
            break;
        default:
            break;
    }
    return true;
}

#include <X11/X.h>
#include <X11/keysym.h>

int TranslateXKeyToVKey(KeySym key)
{
    if (key == XK_Pointer_Button1) return VK_LBUTTON;
    if (key == XK_Pointer_Button2) return VK_RBUTTON;
    if (key == XK_Cancel) return VK_CANCEL;
    if (key == XK_Pointer_Button3) return VK_MBUTTON;
    if (key == XK_BackSpace) return VK_BACK;
    if (key == XK_Tab) return VK_TAB;
    if (key == XK_Clear) return VK_CLEAR;
    if (key == XK_Return) return VK_RETURN;
    //if (key == XK_) return VK_SHIFT;
    //if (key == XK_) return VK_CONTROL;
    if (key == XK_Menu) return VK_MENU;
    if (key == XK_Pause) return VK_PAUSE;
    if (key == XK_Caps_Lock) return VK_CAPITAL;
    if (key == XK_Escape) return VK_ESCAPE;
    if (key == XK_space) return VK_SPACE;
    if (key == XK_Prior) return VK_PRIOR;
    if (key == XK_Next) return VK_NEXT;
    if (key == XK_End) return VK_END;
    if (key == XK_Home) return VK_HOME;
    if (key == XK_Left) return VK_LEFT;
    if (key == XK_Up) return VK_UP;
    if (key == XK_Right) return VK_RIGHT;
    if (key == XK_Down) return VK_DOWN;
    if (key == XK_Select) return VK_SELECT;
    if (key == XK_Print) return VK_PRINT;
    if (key == XK_Execute) return VK_EXECUTE;
    if (key == XK_Sys_Req) return VK_SNAPSHOT;
    if (key == XK_Insert) return VK_INSERT;
    if (key == XK_Delete) return VK_DELETE;
    if (key == XK_Help) return VK_HELP;
    if (key == XK_KP_0) return VK_NUMPAD0;
    if (key == XK_KP_1) return VK_NUMPAD1;
    if (key == XK_KP_2) return VK_NUMPAD2;
    if (key == XK_KP_3) return VK_NUMPAD3;
    if (key == XK_KP_4) return VK_NUMPAD4;
    if (key == XK_KP_5) return VK_NUMPAD5;
    if (key == XK_KP_6) return VK_NUMPAD6;
    if (key == XK_KP_7) return VK_NUMPAD7;
    if (key == XK_KP_8) return VK_NUMPAD8;
    if (key == XK_KP_9) return VK_NUMPAD9;
    if (key == XK_KP_Separator) return VK_SEPARATOR;
    if (key == XK_KP_Subtract) return VK_SUBTRACT;
    if (key == XK_KP_Decimal) return VK_DECIMAL;
    if (key == VK_DIVIDE) return VK_DIVIDE;
    if (key == XK_F1) return VK_F1;
    if (key == XK_F2) return VK_F2;
    if (key == XK_F3) return VK_F3;
    if (key == XK_F4) return VK_F4;
    if (key == XK_F5) return VK_F5;
    if (key == XK_F6) return VK_F6;
    if (key == XK_F7) return VK_F7;
    if (key == XK_F8) return VK_F8;
    if (key == XK_F9) return VK_F9;
    if (key == XK_F10) return VK_F10;
    if (key == XK_F11) return VK_F11;
    if (key == XK_F12) return VK_F12;
    if (key == XK_F13) return VK_F13;
    if (key == XK_F14) return VK_F14;
    if (key == XK_F15) return VK_F15;
    if (key == XK_F16) return VK_F16;
    if (key == XK_F17) return VK_F17;
    if (key == XK_F18) return VK_F18;
    if (key == XK_F19) return VK_F19;
    if (key == XK_F20) return VK_F20;
    if (key == XK_F21) return VK_F21;
    if (key == XK_F22) return VK_F22;
    if (key == XK_F23) return VK_F23;
    if (key == XK_F24) return VK_F24;
    if (key == XK_Num_Lock) return VK_NUMLOCK;
    if (key == XK_Scroll_Lock) return VK_SCROLL;
    if (key == XK_Shift_L) return VK_LSHIFT;
    if (key == XK_Shift_R) return VK_RSHIFT;
    if (key == XK_Control_L) return VK_LCONTROL;
    if (key == XK_Control_R) return VK_RCONTROL;
    if (key == XK_Alt_L) return VK_LMENU;
    if (key == XK_Alt_R) return VK_RMENU;
    //if (key == XK_3270_Play) return VK_PLAY;
    //if (key == XK_) return VK_ZOOM;

    return key;
}

#endif
