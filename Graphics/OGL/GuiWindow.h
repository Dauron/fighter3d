#ifndef __incl_Graphics_OGL_GuiWindow_h
#define __incl_Graphics_OGL_GuiWindow_h

#include "Utils.h"
#include "../FontMgr.h"
#include "GuiControl.h"
#include "../../Utils/xString.h"

namespace Graphics { namespace OGL {

    class CGuiWindow : public CGuiControl
    {
    private:
        xString m_sText;

        HFont   m_pFont;

        enum   EMouseOperations
        {
            MO_None    = 0,
            MO_Move    = 1,
            MO_ResizeL = 2,
            MO_ResizeR = 4,
            MO_ResizeT = 8,
            MO_ResizeB = 16,
        };
        EMouseOperations m_nMouseOperation;

    public:
        bool m_bSizeable;
        bool m_bScrollable;

        xColor m_sBackgroundColor;
        xColor m_sHeaderColor;
        xColor m_sBorderColor;

        const xString &GetText() const               { return m_sText; }
        void           SetText(const xString &sText) { m_sText = sText; }

        CGuiWindow()
            : CGuiControl()
            , m_pFont(HFont()), m_nMouseOperation(MO_None) {}
        CGuiWindow(const xString &sText, xSHORT nX, xSHORT nY, xSHORT nWidth, xSHORT nHeight);
        virtual ~CGuiWindow()
        { g_FontMgr.Release(m_pFont); }

        virtual void OnMouseLeftDown(bool receiver);
        virtual void OnMouseMove    (bool receiver, xSHORT nX, xSHORT nY);
        virtual void OnMouseLeftUp  (bool receiver)
        {
            m_nMouseOperation = MO_None;
        }

        virtual void OnRender();
    };

} } // namespace Graphics::OGL

#endif // __incl_Graphics_OGL_GuiWindow_h
