#ifndef __incl_Graphics_OGL_GuiControl_h
#define __incl_Graphics_OGL_GuiControl_h

#include "Utils.h"
#include "../../Utils/Delegate.h"
#include <list>

namespace Graphics { namespace OGL {

    class CGuiControl;
    typedef std::list<CGuiControl*> TLstGuiControlP;

    class CGuiControl
    {
    protected:
        TLstGuiControlP m_lstControls;

        static xSHORT m_nMouseX;
        static xSHORT m_nMouseY;

        enum EButtonState
        {
            BS_Normal    = 0,
            BS_Hover     = 1,
            BS_Down      = 2,
            BS_Disabled  = 3,

            BS_Count     = 4,
        };
        EButtonState m_nState;

        bool m_bVisible;
        bool m_bHitTested;

        EButtonState GetState()
        {
            if (!m_pcParent) return m_nState;

            return static_cast<EButtonState>( max(m_nState, m_pcParent->GetState()) );
        }

        void NotifyVisibilityChanged()
        {
            OnVisibleChanged();

            TLstGuiControlP::reverse_iterator
                CTRL_curr = m_lstControls.rbegin(),
                CTRL_last = m_lstControls.rend();
            for(; CTRL_curr != CTRL_last; ++CTRL_curr)
                if ((**CTRL_curr).m_bVisible)
                    (**CTRL_curr).NotifyVisibilityChanged();
        }

        virtual void ChildIsRemoving(const CGuiControl &pcChild) {}

    public:
        typedef Delegate<CGuiControl> ControlEvent;

        CGuiControl *m_pcParent;
        xSHORT m_nX, m_nY, m_nWidth, m_nHeight;

        xSHORT m_nMarginLeft;
        xSHORT m_nMarginRight;
        xSHORT m_nMarginTop;
        xSHORT m_nMarginBottom;

        void SetEnabled(bool bEnabled)
        {
            if ( bEnabled ^ (m_nState != BS_Disabled) )
            {
                m_nState = (bEnabled) ? BS_Normal : BS_Disabled;
                OnEnabledChanged();
            }
        }
        bool IsEnabled()
        { return m_nState != BS_Disabled; }
        ControlEvent OnEnabledChanged;

        void SetVisible(bool bVisible)
        {
            if (bVisible != m_bVisible)
            {
                m_bVisible = bVisible;
                if (!m_pcParent || m_pcParent->IsVisible())
                    NotifyVisibilityChanged();
            }
        }
        bool IsVisible()
        { return m_bVisible && (!m_pcParent || m_pcParent->IsVisible()); }
        ControlEvent OnVisibleChanged;

        void SetHitTested(bool bHitTested)
        {
            if (bHitTested != m_bHitTested)
            {
                m_bHitTested = bHitTested;
                OnHitTestedChanged();
            }
        }
        bool IsHitTested()
        { return m_bHitTested; }
        ControlEvent OnHitTestedChanged;

        CGuiControl()
            : m_nState(BS_Normal), m_bVisible(true), m_bHitTested(true), m_pcParent(0)
            , m_nX(0), m_nY(0) , m_nWidth(0), m_nHeight(0)
            , m_nMarginLeft(0), m_nMarginRight(0), m_nMarginTop(0), m_nMarginBottom(0)
        {
            OnEnabledChanged   = ControlEvent(*this);
            OnVisibleChanged   = ControlEvent(*this);
            OnHitTestedChanged = ControlEvent(*this);
        }
        CGuiControl(xSHORT nX, xSHORT nY, xSHORT nWidth, xSHORT nHeight)
            : m_nState(BS_Normal), m_bVisible(true), m_bHitTested(true), m_pcParent(0)
            , m_nX(nX), m_nY(nY) , m_nWidth(nWidth), m_nHeight(nHeight)
            , m_nMarginLeft(0), m_nMarginRight(0), m_nMarginTop(0), m_nMarginBottom(0)
        {
            OnEnabledChanged   = ControlEvent(*this);
            OnVisibleChanged   = ControlEvent(*this);
            OnHitTestedChanged = ControlEvent(*this);
        }
        virtual ~CGuiControl()
        {
            ClearChildren();
        }

        void AddChild(CGuiControl &control)
        {
            control.m_pcParent = this;
            m_lstControls.remove   (&control);
            m_lstControls.push_back(&control);
        }
        void RemoveChild(CGuiControl &control, bool bFree = true)
        {
            if (control.m_pcParent == this)
            {
                ChildIsRemoving(control);
                control.m_pcParent = 0;
                m_lstControls.remove(&control);
                if (bFree) delete &control;
            }
        }
        void ClearChildren()
        {
            TLstGuiControlP::iterator CTRL_curr = m_lstControls.begin(),
                                      CTRL_last = m_lstControls.end();
            for(; CTRL_curr != CTRL_last; ++CTRL_curr)
            {
                ChildIsRemoving(**CTRL_curr);
                delete *CTRL_curr;
            }
        }

        void LocalToChild(xSHORT &nX, xSHORT &nY)
        {
            nX -= m_nX + m_nMarginLeft;
            nY -= m_nY + m_nMarginTop;
        }
        void GlobalToLocal(xSHORT &nX, xSHORT &nY)
        {
            if (m_pcParent)
            {
                m_pcParent->GlobalToLocal(nX, nY);
                m_pcParent->LocalToChild (nX, nY);
            }
        }
        virtual bool HitTest(xSHORT nX, xSHORT nY)
        {
            return (nX >= m_nX && nY >= m_nY &&
                    nX <= m_nX + m_nWidth    &&
                    nY <= m_nY + m_nHeight   );
        }

        CGuiControl *MouseMove(xSHORT nX, xSHORT nY);

        void         MouseLeftDown    (bool receiver = true)
        {
            OnMouseLeftDown(receiver);
            if (m_pcParent)
                m_pcParent->MouseLeftDown(false);
        }
        void         MouseLeftUp    (bool receiver = true)
        {
            OnMouseLeftUp(receiver);
            if (m_pcParent)
                m_pcParent->MouseLeftUp(false);
        }

        virtual void OnMouseLeftDown(bool receiver) {}
        virtual void OnMouseLeftUp  (bool receiver) {}
        virtual void OnMouseMove    (bool receiver, xSHORT nX, xSHORT nY) {}
        virtual void OnMouseEnter   () {}
        virtual void OnMouseLeave   () {}

        void Render();
        virtual void OnRender() {}

        virtual void SetScissor(xSHORT nX, xSHORT nY, xSHORT nWidth, xSHORT nHeight)
        {
            nX += m_nMarginLeft;
            nY += m_nMarginTop;

            if (nX < 0) nX = 0;
            if (nY < 0) nY = 0;

            if (nX + nWidth > m_nWidth - m_nMarginRight)
                nWidth = m_nWidth - nX - m_nMarginRight;
            if (nWidth < 0) nWidth = 0;

            if (nY + nHeight > m_nHeight - m_nMarginBottom)
                nHeight = m_nHeight - nY - m_nMarginBottom;
            if (nHeight < 0) nHeight = 0;

            nX += m_nX;
            nY += m_nY;

            if (m_pcParent)
                m_pcParent->SetScissor(nX, nY, nWidth, nHeight);
        }
    };

} } // namespace Graphics::OGL

#endif // __incl_Graphics_OGL_GuiControl_h
