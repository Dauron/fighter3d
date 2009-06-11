#include "GuiCheckBox.h"
#include "GuiManager.h"

using namespace Graphics::OGL;

void CGuiCheckBox::Zero()
{
    OnCheckedChanged = ControlEvent(*this);

    m_hBoxIcon  = HTexture();
    m_hTickIcon = HTexture();
    m_pcLabel   = 0;
    m_nBoxX     = m_nBoxY  = m_nBoxWidth  = m_nBoxHeight  = 0;
    m_nTickX    = m_nTickY = m_nTickWidth = m_nTickHeight = 0;
    m_bChecked  = false;
    m_sBackgroundColor.init(0.3f, 0.3f, 0.8f, 0.0f);
    m_sBorderColor    .init(0.5f, 0.5f, 1.0f, 0.0f);
}

void CGuiCheckBox::SetImage(const char *sTickIcon,        xSHORT nTickWidth /*= 0*/, xSHORT nTickHeight /*= 0*/,
                            const char *sBoxIcon /*= 0*/, xSHORT nBoxWidth  /*= 0*/, xSHORT nBoxHeight  /*= 0*/)
{
    g_TextureMgr.Release(m_hBoxIcon);
    g_TextureMgr.Release(m_hTickIcon);

    m_hBoxIcon    = sBoxIcon    ? g_TextureMgr.GetTexture(sBoxIcon)  : HTexture();
    m_hTickIcon   = sTickIcon   ? g_TextureMgr.GetTexture(sTickIcon) : HTexture();
    m_nTickWidth  = nTickWidth  ? nTickWidth  : m_nHeight;
    m_nTickHeight = nTickHeight ? nTickHeight : m_nHeight;
    m_nBoxWidth   = nBoxWidth   ? nBoxWidth   : m_nHeight;
    m_nBoxHeight  = nBoxHeight  ? nBoxHeight  : m_nHeight;

    m_nBoxX  = 0;
    m_nBoxY  = (m_nHeight    - m_nBoxHeight ) >> 1;
    m_nTickX = (m_nBoxWidth  - m_nTickWidth ) >> 1;
    m_nTickY = (m_nBoxHeight - m_nTickHeight) >> 1;

    if (m_pcLabel) m_pcLabel->m_nX = m_nBoxX + m_nBoxWidth;
}

void CGuiCheckBox::SetText(const xString &sText,
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
            AddChild(*(m_pcLabel = new CGuiLabel(sText, m_nBoxX + m_nBoxWidth, 0, m_nWidth, m_nHeight)));
            m_pcLabel->SetHitTested(false);
        }
        else
            m_pcLabel->SetText(sText);

        m_pcLabel->SetFont(sFont, nSize);
    }
}

void CGuiCheckBox::OnMouseLeftDown(bool receiver)
{
    if (!receiver) return;
    if (m_nState != BS_Disabled)
    {
        m_nState = BS_Down;
        SetChecked(!m_bChecked);
    }
}

void CGuiCheckBox::OnMouseLeftUp(bool receiver)
{
    if (!receiver) return;
    if (m_nState != BS_Disabled)
        m_nState = BS_Hover;
}

void CGuiCheckBox::OnMouseEnter()
{
    if (m_nState != BS_Disabled && m_nState != BS_Down)
        m_nState = BS_Hover;
}

void CGuiCheckBox::OnMouseLeave()
{
    if (m_nState != BS_Disabled)
        m_nState = BS_Normal;
}

void CGuiCheckBox::OnRender()
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

    if (!m_hBoxIcon.IsNull())
    {
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glColor3ub(255, 255, 255);
        int nState = static_cast<int>( GetState() );

        g_TextureMgr.BindTexture(m_hBoxIcon);
        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.25f* nState   , 1.f); glVertex2i(m_nX + m_nBoxX              , m_nY + m_nBoxY);
            glTexCoord2f(0.25f*(nState+1), 1.f); glVertex2i(m_nX + m_nBoxX + m_nBoxWidth, m_nY + m_nBoxY);
            glTexCoord2f(0.25f*(nState+1), 0.f); glVertex2i(m_nX + m_nBoxX + m_nBoxWidth, m_nY + m_nBoxY + m_nBoxHeight);
            glTexCoord2f(0.25f* nState   , 0.f); glVertex2i(m_nX + m_nBoxX              , m_nY + m_nBoxY + m_nBoxHeight);
        }
        glEnd();

        if (m_bChecked && !m_hTickIcon.IsNull())
        {
            g_TextureMgr.BindTexture(m_hTickIcon);
            glBegin(GL_QUADS);
            {
                glTexCoord2f(0.25f* nState   , 1.f); glVertex2i(m_nX + m_nTickX               , m_nY + m_nTickY);
                glTexCoord2f(0.25f*(nState+1), 1.f); glVertex2i(m_nX + m_nTickX + m_nTickWidth, m_nY + m_nTickY);
                glTexCoord2f(0.25f*(nState+1), 0.f); glVertex2i(m_nX + m_nTickX + m_nTickWidth, m_nY + m_nTickY + m_nTickHeight);
                glTexCoord2f(0.25f* nState   , 0.f); glVertex2i(m_nX + m_nTickX               , m_nY + m_nTickY + m_nTickHeight);
            }
            glEnd();
        }

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
