#include "GuiButton.h"
#include "GuiManager.h"

using namespace Graphics::OGL;

void CGuiButton::Zero()
{
    m_hIcon      = HTexture();
    m_pcLabel    = 0;
    m_nIconWidth = m_nIconHeight = 0;

    m_sBackgroundColor.init(0.3f, 0.3f, 0.8f, 0.8f);
    m_sBorderColor    .init(0.5f, 0.5f, 1.0f, 1.0f);
}

void CGuiButton::SetImage(const xString &sIcon   /*= xString::Empty()*/,
                                xSHORT   nWidth  /*= 0*/,
                                xSHORT   nHeight /*= 0*/)
{
    g_TextureMgr.Release(m_hIcon);

    m_hIcon       = (sIcon) ? g_TextureMgr.GetTexture(sIcon.cstr()) : HTexture();
    m_nIconWidth  = nWidth  ? nWidth  : m_nWidth;
    m_nIconHeight = nHeight ? nHeight : m_nHeight;

    if (m_pcLabel)
        m_pcLabel->m_nX = m_nIconWidth;
}

void CGuiButton::SetText(const xString &sText /*= xString::Empty()*/,
                         const xString &sFont /*= 0*/,
                               xSHORT   nSize /*= 0*/)
{
    if(sText.empty())
    {
        if (m_pcLabel)
            RemoveChild(*m_pcLabel);
    }
    else
    {
        if (!m_pcLabel)
        {
            AddChild(*(m_pcLabel = new CGuiLabel(sText, m_nIconWidth, 0, m_nWidth, m_nHeight)));
            m_pcLabel->SetHitTested(false);
        }
        else
            m_pcLabel->SetText(sText);

        m_pcLabel->SetFont(sFont, nSize);
    }
}

void CGuiButton::OnMouseLeftDown(bool receiver)
{
    if (!receiver) return;
    if (m_nState != BS_Disabled)
        m_nState = BS_Down;
}

void CGuiButton::OnMouseLeftUp(bool receiver)
{
    if (!receiver) return;
    if (m_nState != BS_Disabled)
        m_nState = BS_Hover;
}

void CGuiButton::OnMouseEnter()
{
    if (m_nState != BS_Disabled && m_nState != BS_Down)
        m_nState = BS_Hover;
}

void CGuiButton::OnMouseLeave()
{
    if (m_nState != BS_Disabled)
        m_nState = BS_Normal;
}

void CGuiButton::OnRender()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // Background
    if (m_sBackgroundColor.a)
    {
        if (m_sBackgroundColor.a < 1.f)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
        glColor4fv(m_sBackgroundColor.rgba);
        glRecti( m_nX, m_nY, m_nX + m_nWidth, m_nY + m_nHeight);
    }

    xSHORT nIconXY = 0;
    if (!m_hIcon.IsNull())
    {
        glEnable(GL_BLEND);
        nIconXY = (m_nHeight - m_nIconHeight) >> 1;
        glEnable(GL_TEXTURE_2D);
        g_TextureMgr.BindTexture(m_hIcon);
        glColor3ub(255, 255, 255);
        glBegin(GL_QUADS);
        {
            int nState = static_cast<int>( GetState() );
            glTexCoord2f(0.25f* nState   , 1.f); glVertex2i(m_nX + nIconXY               , m_nY + nIconXY);
            glTexCoord2f(0.25f*(nState+1), 1.f); glVertex2i(m_nX + nIconXY + m_nIconWidth, m_nY + nIconXY);
            glTexCoord2f(0.25f*(nState+1), 0.f); glVertex2i(m_nX + nIconXY + m_nIconWidth, m_nY + nIconXY + m_nIconHeight);
            glTexCoord2f(0.25f* nState   , 0.f); glVertex2i(m_nX + nIconXY               , m_nY + nIconXY + m_nIconHeight);
        }
        glEnd();

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }

    // Border
    if (m_sBorderColor.a)
    {
        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        if (m_sBorderColor.a < 1.f)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
        glColor4fv(m_sBorderColor.rgba);
        glRecti( m_nX, m_nY, m_nX + m_nWidth, m_nY + m_nHeight);
    }
}
