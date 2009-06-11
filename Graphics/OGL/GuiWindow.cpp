#include "GuiWindow.h"
#include "GuiManager.h"

using namespace Graphics::OGL;

CGuiWindow::CGuiWindow(const xString &sText, xSHORT nX, xSHORT nY, xSHORT nWidth, xSHORT nHeight)
            : CGuiControl(nX, nY, nWidth, nHeight)
            , m_pFont(HFont()), m_nMouseOperation(MO_None)
{
    m_sText = sText;
    m_sBackgroundColor.init(0.3f, 0.3f, 0.8f, 0.8f);
    m_sHeaderColor    .init(0.0f, 1.0f, 0.3f, 0.8f);
    m_sBorderColor    .init(0.5f, 0.5f, 1.0f, 1.0f);

    if ( m_pFont.IsNull() )
        m_pFont = g_FontMgr.GetFont("Arial", 12);
    const Font *pFont = g_FontMgr.GetFont(m_pFont);

    m_nMarginLeft   =
    m_nMarginRight  =
    m_nMarginBottom = 5;
    m_nMarginTop    = 5 + static_cast<xSHORT>( ceilf(pFont->GlyphHeight * 1.1f) );

    m_bSizeable   = true;
    m_bScrollable = false;
}

void CGuiWindow::OnMouseLeftDown(bool receiver)
{
    if (m_pcParent)
        m_pcParent->AddChild(*this); // move to front

    if (!receiver) return;

    m_nMouseOperation = MO_None;

    if (m_nMouseX - m_nX < m_nMarginLeft)
        m_nMouseOperation = MO_ResizeL;
    else
    if (m_nX + m_nWidth - m_nMouseX < m_nMarginRight)
        m_nMouseOperation = MO_ResizeR;

    if (m_nMouseY - m_nY < 5)
        m_nMouseOperation = static_cast<EMouseOperations>( m_nMouseOperation | MO_ResizeT );
    else
    if (m_nY + m_nHeight - m_nMouseY < m_nMarginBottom)
        m_nMouseOperation = static_cast<EMouseOperations>( m_nMouseOperation | MO_ResizeB );

    if (m_nMouseOperation == MO_None)
        m_nMouseOperation = MO_Move;
}

void CGuiWindow::OnMouseMove(bool receiver, xSHORT nX, xSHORT nY)
{
    if (!receiver) return;

    if (m_nMouseOperation != MO_None)
    {
        xSHORT nXDiff = nX - m_nMouseX;
        xSHORT nYDiff = nY - m_nMouseY;

        if (m_nMouseOperation == MO_Move)
        {
            m_nX += nXDiff;
            m_nY += nYDiff;
        }
        else
        {
            if (m_nMouseOperation & MO_ResizeL)
            {
                m_nX     += nXDiff;
                m_nWidth -= nXDiff;
            }
            else
            if (m_nMouseOperation & MO_ResizeR)
            {
                m_nWidth += nXDiff;
                if (m_nWidth < 10) m_nX -= 10 - m_nWidth;
            }

            if (m_nMouseOperation & MO_ResizeT)
            {
                m_nY      += nYDiff;
                m_nHeight -= nYDiff;
            }
            else
            if (m_nMouseOperation & MO_ResizeB)
            {
                m_nHeight += nYDiff;
                if (m_nHeight < 10) m_nY -= 10 - m_nHeight;
            }

            if (m_nWidth  < 10) m_nWidth  = 10;
            if (m_nHeight < 10) m_nHeight = 10;
        }

    }
}

void CGuiWindow::OnRender()
{
    const Font *pFont = g_FontMgr.GetFont(m_pFont);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // Background
    if (m_sBackgroundColor.a < 1.f)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
    glColor4fv(m_sBackgroundColor.rgba);
    glRecti( m_nX+m_nMarginLeft, m_nY+m_nMarginTop, m_nX + m_nWidth - m_nMarginRight, m_nY + m_nHeight - m_nMarginBottom);

    // Header
    if (m_sHeaderColor.a < 1.f)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
    glColor4fv(m_sHeaderColor.rgba);
    glRecti( m_nX+m_nMarginLeft, m_nY + 5, m_nX + m_nWidth - m_nMarginRight, m_nY + m_nMarginTop);

    glColor3f(1.f, 1.f, 1.f);
    pFont->Print(static_cast<xFLOAT>( m_nX+m_nMarginLeft+3      ),
                 static_cast<xFLOAT>( m_nY+6+pFont->GlyphAscent ),
                 0.f, m_sText.cstr());

    // Border
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    if (m_sBorderColor.a < 1.f)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
    glColor4fv(m_sBorderColor.rgba);
    glRecti( m_nX, m_nY, m_nX + m_nWidth, m_nY + m_nHeight);
}
