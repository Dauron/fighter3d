#include "GuiSlider.h"
#include "GuiManager.h"

using namespace Graphics::OGL;

void CGuiSlider::SetImage(const char *sIconSlider /*= 0*/, xSHORT nSliderWidth /*= 0*/, xSHORT nSliderHeight /*= 0*/,
                          const char *sIconBox    /*= 0*/, xSHORT nBoxWidth    /*= 0*/, xSHORT nBoxHeight    /*= 0*/ )
{
    g_TextureMgr.Release(m_pIconBox);
    g_TextureMgr.Release(m_pIconSlider);

    m_pIconBox      = (sIconBox)    ? g_TextureMgr.GetTexture(sIconBox)  : HTexture();
    m_pIconSlider   = (sIconSlider) ? g_TextureMgr.GetTexture(sIconSlider) : HTexture();
    m_nSliderWidth  = nSliderWidth  ? nSliderWidth  : 32;
    m_nSliderHeight = nSliderHeight ? nSliderHeight : m_nHeight;
    m_nBoxWidth     = nBoxWidth     ? nBoxWidth     : m_nWidth - m_nSliderWidth;
    m_nBoxHeight    = nBoxHeight    ? nBoxHeight    : m_nHeight;

    m_nBoxX = (m_nWidth  - m_nBoxWidth)  >> 1;
    m_nBoxY = (m_nHeight - m_nBoxHeight) >> 1;
}

void CGuiSlider::OnMouseLeftDown(bool receiver)
{
    if (!receiver) return;
    if (m_nState != BS_Disabled)
    {
        m_nState = BS_Down;
        SetValuePercent( (m_nMouseX - m_nX - m_nBoxX) / static_cast<xFLOAT>( m_nBoxWidth ) );
    }
}

void CGuiSlider::OnMouseLeftUp(bool receiver)
{
    if (!receiver) return;
    if (m_nState != BS_Disabled)
        m_nState = BS_Hover;
}

void CGuiSlider::OnMouseMove(bool receiver, xSHORT nX, xSHORT nY)
{
    if (!receiver) return;
    if (m_nState == BS_Down)
        SetValuePercent( (m_nMouseX - m_nX - m_nBoxX) / static_cast<xFLOAT>( m_nBoxWidth ) );
}

void CGuiSlider::OnMouseEnter()
{
    if (m_nState != BS_Disabled && m_nState != BS_Down)
        m_nState = BS_Hover;
}

void CGuiSlider::OnMouseLeave()
{
    if (m_nState != BS_Disabled)
        m_nState = BS_Normal;
}

void CGuiSlider::OnRender()
{
    if (!m_pIconBox.IsNull())
    {
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glColor3ub(255, 255, 255);
        int nState = static_cast<int>( m_nState );

        g_TextureMgr.BindTexture(m_pIconBox);
        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.f, 0.25f*(nState+1)); glVertex2i(m_nX + m_nBoxX              , m_nY + m_nBoxY);
            glTexCoord2f(1.f, 0.25f*(nState+1)); glVertex2i(m_nX + m_nBoxX + m_nBoxWidth, m_nY + m_nBoxY);
            glTexCoord2f(1.f, 0.25f* nState   ); glVertex2i(m_nX + m_nBoxX + m_nBoxWidth, m_nY + m_nBoxY + m_nBoxHeight);
            glTexCoord2f(0.f, 0.25f* nState   ); glVertex2i(m_nX + m_nBoxX              , m_nY + m_nBoxY + m_nBoxHeight);
        }
        glEnd();

        if (!m_pIconSlider.IsNull())
        {
            g_TextureMgr.BindTexture(m_pIconSlider);
            glBegin(GL_QUADS);
            {
                xFLOAT fPercent = m_fCurValue / (m_fMaxValue - m_fMinValue);
                xSHORT m_nSliderX = m_nBoxX + static_cast<xSHORT>( fPercent * m_nBoxWidth ) - (m_nSliderWidth >> 1);
                xSHORT m_nSliderY = m_nY + ((m_nHeight - m_nSliderHeight) >> 1);
                glTexCoord2f(0.25f* nState   , 1.f); glVertex2i(m_nX + m_nSliderX                 , m_nSliderY);
                glTexCoord2f(0.25f*(nState+1), 1.f); glVertex2i(m_nX + m_nSliderX + m_nSliderWidth, m_nSliderY);
                glTexCoord2f(0.25f*(nState+1), 0.f); glVertex2i(m_nX + m_nSliderX + m_nSliderWidth, m_nSliderY + m_nSliderHeight);
                glTexCoord2f(0.25f* nState   , 0.f); glVertex2i(m_nX + m_nSliderX                 , m_nSliderY + m_nSliderHeight);
            }
            glEnd();
        }

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
}
