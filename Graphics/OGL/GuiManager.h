#ifndef __incl_Graphics_OGL_GuiManager_h
#define __incl_Graphics_OGL_GuiManager_h

#include "../../AppFramework/Application.h"
#include "GuiControl.h"

namespace Graphics { namespace OGL {

    class CGuiManager : public CGuiControl
    {
        CGuiControl *m_pWhoCapturedMouse;
        CGuiControl *m_pWhoWasLastHit;
        bool         m_bMouseLDown;

        bool         m_bMouseHiddenForGui;

    public:

        CGuiManager()
            : CGuiControl(), m_pWhoCapturedMouse(0), m_pWhoWasLastHit(0)
            , m_bMouseLDown(false), m_bMouseHiddenForGui(false)
        {}
        CGuiManager(xSHORT nX, xSHORT nY, xSHORT nWidth, xSHORT nHeight)
            : CGuiControl(nX, nY, nWidth, nHeight)
            , m_pWhoCapturedMouse(0), m_pWhoWasLastHit(0)
            , m_bMouseLDown(false), m_bMouseHiddenForGui(false)
        {}

        void Resize(xSHORT nX, xSHORT nY, xSHORT nWidth, xSHORT nHeight)
        {
            m_nX      = nX;
            m_nY      = nY;
            m_nWidth  = nWidth;
            m_nHeight = nHeight;
        }

        void MouseHideFromGui(bool bHide)
        {
            if (bHide != m_bMouseHiddenForGui)
            {
                if (bHide)
                    m_bMouseHiddenForGui = true;
                else
                {
                    m_bMouseHiddenForGui = false;
                    MouseUpdate();
                }
            }
        }

        virtual bool HitTest(xSHORT nX, xSHORT nY)
        {
            if (m_bMouseHiddenForGui) return false;

            LocalToChild(nX, nY);

            TLstGuiControlP::iterator CTRL_curr = m_lstControls.begin(),
                                      CTRL_last = m_lstControls.end();
            for(; CTRL_curr != CTRL_last; ++CTRL_curr)
                if ((**CTRL_curr).HitTest(nX, nY))
                    return true;

            return false;
        }

        bool MouseUpdate()
        {
            if (m_bMouseHiddenForGui) return false;

            bool bCaptured = false;
            if (g_InputMgr.KeyDown_Get(VK_LBUTTON))
            {
                if (!m_bMouseLDown)
                {
                    m_bMouseLDown       = true;

                    m_pWhoCapturedMouse = m_pWhoWasLastHit;
                    if (m_pWhoCapturedMouse)
                        m_pWhoCapturedMouse->MouseLeftDown();
                }
                bCaptured = m_pWhoCapturedMouse;
            }
            else
            {
                if (m_bMouseLDown)
                {
                    m_bMouseLDown = false;

                    if (m_pWhoCapturedMouse)
                    {
                        bCaptured = true;
                        m_pWhoCapturedMouse->MouseLeftUp();
                        m_pWhoCapturedMouse = 0;
                    }
                }
            }

            if (bCaptured                      ||
                m_nMouseX != g_InputMgr.mouseX ||
                m_nMouseY != g_InputMgr.mouseY )
            {
                xSHORT nX = g_InputMgr.mouseX;
                xSHORT nY = g_InputMgr.mouseY;

                if (m_pWhoCapturedMouse)
                {
                    m_pWhoCapturedMouse->GlobalToLocal(nX, nY);
                    m_pWhoCapturedMouse->OnMouseMove  (true, nX, nY);
                    m_nMouseX = nX;
                    m_nMouseY = nY;
                }
                else
                {
                    LocalToChild(nX, nY);
                    CGuiControl *pWhoWasHit = MouseMove(nX, nY);
                    if (m_pWhoWasLastHit != pWhoWasHit)
                    {
                        if (m_pWhoWasLastHit) m_pWhoWasLastHit->OnMouseLeave();
                        m_pWhoWasLastHit = pWhoWasHit;
                        if (m_pWhoWasLastHit) m_pWhoWasLastHit->OnMouseEnter();
                    }

                }
            }
            return bCaptured;
        }

        virtual void OnRender()
        {
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(m_nX, m_nY+m_nWidth, m_nY+m_nHeight, m_nY, 0, 100);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glViewport(m_nX, m_nY, m_nWidth, m_nHeight);
        }

        virtual void SetScissor(xSHORT nX, xSHORT nY, xSHORT nWidth, xSHORT nHeight)
        {
            LocalToChild(nX, nY);
            nY = g_Application.MainWindow_Get().Height_Get() - nY - nHeight;

            glScissor(nX, nY, nWidth, nHeight);
            glEnable (GL_SCISSOR_TEST);
        }
    };

} } // namespace Graphics::OGL

#endif // __incl_Graphics_OGL_GuiManager_h
