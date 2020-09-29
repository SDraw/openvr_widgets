#include "stdafx.h"

#include "Widgets/Widget.h"
#include "Utils/Transformation.h"

Widget::Widget()
{
    m_overlay = vr::k_ulOverlayHandleInvalid;
    m_texture = { 0 };
    m_texture.eType = vr::TextureType_OpenGL;
    m_texture.eColorSpace = vr::ColorSpace_Gamma;
    m_event = { 0 };

    m_valid = false;
    m_visible = false;
    m_closed = false;
    m_activeDashboard = false;
    m_transform = new Transformation();
}
Widget::~Widget()
{
    delete m_transform;
}

void Widget::Destroy()
{
    if(m_overlay != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->HideOverlay(m_overlay);
        vr::VROverlay()->ClearOverlayTexture(m_overlay);
        vr::VROverlay()->DestroyOverlay(m_overlay);
        m_overlay = vr::k_ulOverlayHandleInvalid;
    }

    m_texture.handle = nullptr;

    m_valid = false;
    m_visible = false;
    m_closed = false;
    m_activeDashboard = false;
}

bool Widget::IsClosed() const
{
    return m_closed;
}

void Widget::OnHandActivated(size_t f_hand)
{
}
void Widget::OnHandDeactivated(size_t f_hand)
{
}
void Widget::OnButtonPress(size_t f_hand, uint32_t f_button)
{
}
void Widget::OnButtonRelease(size_t f_hand, uint32_t f_button)
{
}

void Widget::OnDashboardOpen()
{
    if(m_valid) m_activeDashboard = true;
}
void Widget::OnDashboardClose()
{
    if(m_valid) m_activeDashboard = false;
}
