#ifndef __incl_Graphics_OGL_GuiButton_h
#define __incl_Graphics_OGL_GuiButton_h

#include "../Textures/TextureMgr.h"
#include "GuiLabel.h"

namespace Graphics { namespace OGL {

    class CGuiButton : public CGuiControl
    {
    private:
        HTexture    m_hIcon;

        CGuiLabel * m_pcLabel;

        xSHORT m_nIconWidth, m_nIconHeight;

        void Zero();

    protected:
        virtual void ChildIsRemoving(const CGuiControl &pcChild)
        {
            if (&pcChild == m_pcLabel)
                m_pcLabel = 0;
        }

    public:
        xColor m_sBackgroundColor;
        xColor m_sBorderColor;

        void SetImage(const xString &sIcon   = xString::Empty(),
                            xSHORT   nWidth  = 0,
                            xSHORT   nHeight = 0);
        void SetText (const xString &sText,
                      const xString &sFont = xString::Empty(),
                            xSHORT   nSize = 0);

        CGuiButton() : CGuiControl()
        { Zero(); }
        CGuiButton(xSHORT nX, xSHORT nY, xSHORT nWidth, xSHORT nHeight)
            : CGuiControl(nX, nY, nWidth, nHeight)
        { Zero(); }
        virtual ~CGuiButton()
        { g_TextureMgr.Release(m_hIcon); }

        virtual void OnMouseLeftDown(bool receiver);
        virtual void OnMouseLeftUp  (bool receiver);
        virtual void OnMouseEnter   ();
        virtual void OnMouseLeave   ();

        virtual void OnRender();
    };

} } // namespace Graphics::OGL

#endif // __incl_Graphics_OGL_GuiButton_h
