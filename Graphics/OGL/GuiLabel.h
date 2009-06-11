#ifndef __incl_Graphics_OGL_GuiLabel_h
#define __incl_Graphics_OGL_GuiLabel_h

#include "Utils.h"
#include "../FontMgr.h"
#include "GuiControl.h"
#include "../../Utils/xString.h"

namespace Graphics { namespace OGL {

    class CGuiLabel : public CGuiControl
    {
    private:
        xString  m_sText;

        xString  m_sFontFamily;
        xSHORT   m_nFontSize;
        HFont    m_pFont;

        void Zero();

    public:
        xColor m_sStateColor[CGuiControl::BS_Count];

        const xString &GetText() const               { return m_sText; }
        void           SetText(const xString &sText)
        {
            if (m_sText != sText)
            {
                m_sText = sText;
                OnTextChanged();
            }
        }
        ControlEvent OnTextChanged;

        void SetFont(xString sFamily = xString::Empty(),
                     xSHORT  nSize   = 0)
        {
            if ( (!sFamily.empty() && sFamily != m_sFontFamily) ||
                 (nSize && nSize != m_nFontSize) )
            {
                if (!sFamily.empty()) m_sFontFamily = sFamily;
                if (nSize           ) m_nFontSize   = nSize;

                if (!m_pFont.IsNull())
                {
                    g_FontMgr.Release(m_pFont);
                    m_pFont = HFont();
                }

                OnFontChanged();
            }
        }
        ControlEvent OnFontChanged;

        CGuiLabel()
            : CGuiControl()
        { Zero(); }
        CGuiLabel(const xString &sText, xSHORT nX, xSHORT nY, xSHORT nWidth, xSHORT nHeight)
            : CGuiControl(nX, nY, nWidth, nHeight)
        { Zero(); SetText(sText); }
        virtual ~CGuiLabel()
        { g_FontMgr.Release(m_pFont); }

        virtual void OnMouseLeftDown(bool receiver);
        virtual void OnMouseLeftUp  (bool receiver);
        virtual void OnMouseEnter   ();
        virtual void OnMouseLeave   ();

        virtual void OnRender();
    };

} } // namespace Graphics::OGL

#endif // __incl_Graphics_OGL_GuiLabel_h
