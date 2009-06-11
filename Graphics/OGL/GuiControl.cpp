#include "GuiControl.h"

using namespace Graphics::OGL;

xSHORT CGuiControl::m_nMouseX = -1;
xSHORT CGuiControl::m_nMouseY = -1;

CGuiControl *CGuiControl::MouseMove(xSHORT nX, xSHORT nY)
{
    if (!IsVisible() || !HitTest(nX, nY))
        return 0;

    xSHORT nXChild = nX, nYChild = nY;
    LocalToChild(nXChild, nYChild);

    if (nXChild < m_nWidth  - m_nMarginLeft - m_nMarginRight &&
        nYChild < m_nHeight - m_nMarginTop  - m_nMarginBottom)
    {
        TLstGuiControlP::reverse_iterator
            CTRL_curr = m_lstControls.rbegin(),
            CTRL_last = m_lstControls.rend();
        CGuiControl *pcHitControl = 0;
        for(; CTRL_curr != CTRL_last; ++CTRL_curr)
        {
            pcHitControl = (**CTRL_curr).MouseMove(nXChild, nYChild);
            OnMouseMove(false, nX, nY);
            if (pcHitControl)
                return pcHitControl;
        }
    }

    if (!IsHitTested())
        return 0;

    OnMouseMove(true, nX, nY);
    m_nMouseX = nX;
    m_nMouseY = nY;

    return this;
}

void CGuiControl::Render()
{
    if (!IsVisible())
        return;

    if (m_pcParent)
        m_pcParent->SetScissor(m_nX, m_nY, m_nWidth, m_nHeight);

    OnRender();

    xSHORT nXOrigin = 0, nYOrigin = 0;
    LocalToChild(nXOrigin, nYOrigin);

    glTranslatef((GLfloat)-nXOrigin, (GLfloat)-nYOrigin, 0);

    TLstGuiControlP::iterator CTRL_curr = m_lstControls.begin(),
                              CTRL_last = m_lstControls.end();
    for(; CTRL_curr != CTRL_last; ++CTRL_curr)
        (**CTRL_curr).Render();

    glTranslatef((GLfloat)nXOrigin, (GLfloat)nYOrigin, 0);
}
