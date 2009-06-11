#ifndef __incl_Graphics_OGL_GuiSlider_h
#define __incl_Graphics_OGL_GuiSlider_h

#include "Utils.h"
#include "../FontMgr.h"
#include "../Textures/TextureMgr.h"
#include "GuiControl.h"
#include "../../Utils/xString.h"

namespace Graphics { namespace OGL {

    class CGuiSlider : public CGuiControl
    {
    private:
        HTexture m_pIconBox;
        HTexture m_pIconSlider;

        enum EButtonState
        {
            BS_Normal    = 0,
            BS_Hover     = 1,
            BS_Down      = 2,
            BS_Disabled  = 3,
        };
        EButtonState m_nState;

        xSHORT m_nBoxX, m_nBoxY;
        xSHORT m_nSliderWidth, m_nSliderHeight;
        xSHORT m_nBoxWidth,    m_nBoxHeight;

        xFLOAT m_fCurValue;
        xFLOAT m_fMinValue;
        xFLOAT m_fMaxValue;

    public:

        void SetImage(const char *sIconSlider = 0, xSHORT nSliderWidth = 0, xSHORT nSliderHeight = 0,
                      const char *sIconBox    = 0, xSHORT nBoxWidth    = 0, xSHORT nBoxHeight    = 0 );

        void   SetValuePercent(float fAmount)
        {
            SetValue(m_fMinValue + (m_fMaxValue - m_fMinValue) * fAmount);
        }
        void   SetValue(xFLOAT fValue)
        {
            if (fValue < m_fMinValue) fValue = m_fMinValue;
            else
            if (fValue > m_fMaxValue) fValue = m_fMaxValue;
            if (fValue != m_fCurValue)
            {
                m_fCurValue = fValue;
                OnValueChanged();
            }
        }
        xFLOAT GetValue()    { return m_fCurValue; }
        void   SetMinValue(xFLOAT fMinValue)
        {
            if (fMinValue != m_fMinValue)
            {
                m_fMinValue = fMinValue;
                if (m_fMinValue > m_fMaxValue)
                    m_fMaxValue = m_fMinValue;
                SetValue(m_fCurValue);
            }
        }
        xFLOAT GetMinValue() { return m_fMinValue; }
        void   SetMaxValue(xFLOAT fMaxValue)
        {
            if (fMaxValue != m_fMaxValue)
            {
                m_fMaxValue = fMaxValue;
                if (m_fMinValue > m_fMaxValue)
                    m_fMinValue = m_fMaxValue;
                SetValue(m_fCurValue);
            }
        }
        xFLOAT GetMaxValue() { return m_fMaxValue; }

        ControlEvent OnValueChanged;

        CGuiSlider()
            : CGuiControl()
            , m_pIconBox(HTexture()), m_pIconSlider(HTexture())
            , m_nState(BS_Normal), m_fCurValue(0.5f), m_fMinValue(0.f), m_fMaxValue(1.f)
        { OnValueChanged = ControlEvent(*this); }
        CGuiSlider(xSHORT nX, xSHORT nY, xSHORT nWidth, xSHORT nHeight)
            : CGuiControl(nX, nY, nWidth, nHeight)
            , m_pIconBox(HTexture()), m_pIconSlider(HTexture())
            , m_nState(BS_Normal), m_fCurValue(0.5f), m_fMinValue(0.f), m_fMaxValue(1.f)
        { OnValueChanged = ControlEvent(*this); }
        virtual ~CGuiSlider()
        {
            g_TextureMgr.Release(m_pIconBox);
            g_TextureMgr.Release(m_pIconSlider);
        }

        virtual void OnMouseLeftDown(bool receiver);
        virtual void OnMouseLeftUp  (bool receiver);
        virtual void OnMouseMove    (bool receiver, xSHORT nX, xSHORT nY);
        virtual void OnMouseEnter   ();
        virtual void OnMouseLeave   ();

        virtual void OnRender();
    };

} } // namespace Graphics::OGL

#endif // __incl_Graphics_OGL_GuiSlider_h
