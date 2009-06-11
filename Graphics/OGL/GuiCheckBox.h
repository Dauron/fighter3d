#ifndef __incl_Graphics_OGL_GuiCheckBox_h
#define __incl_Graphics_OGL_GuiCheckBox_h

#include "../Textures/TextureMgr.h"
#include "GuiLabel.h"

namespace Graphics { namespace OGL {

    class CGuiCheckBox : public CGuiControl
    {
    private:
        HTexture m_hBoxIcon;
        HTexture m_hTickIcon;

        CGuiLabel * m_pcLabel;

        xSHORT m_nBoxX,  m_nBoxY,  m_nBoxWidth,  m_nBoxHeight;
        xSHORT m_nTickX, m_nTickY, m_nTickWidth, m_nTickHeight;

        bool   m_bChecked;

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

        void SetImage(const char *sTickIcon,    xSHORT nTickWidth = 0, xSHORT nTickHeight = 0,
                      const char *sBoxIcon = 0, xSHORT nBoxWidth  = 0, xSHORT nBoxHeight  = 0 );

        void SetText(const xString &sText,
                     const xString &sFont = xString::Empty(),
                           xSHORT   nSize = 0);

        void SetChecked(bool bChecked)
        {
            if (bChecked != m_bChecked)
            { m_bChecked = bChecked; OnCheckedChanged(); }
        }
        bool IsChecked()
        { return m_bChecked; }
        ControlEvent OnCheckedChanged;

        CGuiCheckBox() : CGuiControl()
        { Zero(); }
        CGuiCheckBox(xSHORT nX, xSHORT nY, xSHORT nWidth, xSHORT nHeight)
            : CGuiControl(nX, nY, nWidth, nHeight)
        { Zero(); }
        virtual ~CGuiCheckBox()
        {
            g_TextureMgr.Release(m_hBoxIcon);
            g_TextureMgr.Release(m_hTickIcon);
        }

        virtual void OnMouseLeftDown(bool receiver);
        virtual void OnMouseLeftUp  (bool receiver);
        virtual void OnMouseEnter   ();
        virtual void OnMouseLeave   ();

        virtual void OnRender();
    };

} } // namespace Graphics::OGL

#endif // __incl_Graphics_OGL_GuiCheckBox_h
