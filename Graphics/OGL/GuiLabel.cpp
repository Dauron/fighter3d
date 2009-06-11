#include "GuiLabel.h"
#include "GuiManager.h"

using namespace Graphics::OGL;

void CGuiLabel::Zero()
{
    OnTextChanged = ControlEvent(*this);
    OnFontChanged = ControlEvent(*this);

    SetFont("Arial", 12);
    m_sStateColor[BS_Normal].init  (0.f ,0.f ,0.f ,1.f);
    m_sStateColor[BS_Hover].init   (0.5f,0.5f,0.f ,1.f);
    m_sStateColor[BS_Down].init    (0.5f,0.f ,0.f ,1.f);
    m_sStateColor[BS_Disabled].init(0.7f,0.7f,0.7f,1.f);
}

void CGuiLabel::OnMouseLeftDown(bool receiver)
{
    if (!receiver) return;
    if (m_nState != BS_Disabled)
        m_nState = BS_Down;
}

void CGuiLabel::OnMouseLeftUp(bool receiver)
{
    if (!receiver) return;
    if (m_nState != BS_Disabled)
        m_nState = BS_Hover;
}

void CGuiLabel::OnMouseEnter()
{
    if (m_nState != BS_Disabled && m_nState != BS_Down)
        m_nState = BS_Hover;
}

void CGuiLabel::OnMouseLeave()
{
    if (m_nState != BS_Disabled)
        m_nState = BS_Normal;
}

void CGuiLabel::OnRender()
{
    if (m_pFont.IsNull())
        m_pFont = g_FontMgr.GetFont(m_sFontFamily.cstr(), m_nFontSize);

    const Font *pFont = g_FontMgr.GetFont(m_pFont);

    glColor4fv(m_sStateColor[GetState()].rgba);
    pFont->Print(static_cast<float>(m_nX + 3),
                 m_nY+(m_nHeight+pFont->GlyphAscent)*0.5f, 0.f, m_sText.cstr());
}
