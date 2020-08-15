#include "stdafx.h"

#include "Widgets/Widget.h"
#include "Utils/Transformation.h"

#include "Utils/GlobalStructures.h"

vr::IVROverlay *Widget::ms_vrOverlay = nullptr;
vr::IVRCompositor *Widget::ms_vrCompositor = nullptr;

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
        ms_vrOverlay->HideOverlay(m_overlay);
        ms_vrOverlay->ClearOverlayTexture(m_overlay);
        ms_vrOverlay->DestroyOverlay(m_overlay);
        m_overlay = vr::k_ulOverlayHandleInvalid;
    }

    m_valid = false;
    m_visible = false;
    m_closed = false;
    m_activeDashboard = false;
}

bool Widget::IsClosed() const
{
    return m_closed;
}

void Widget::OnHandActivated(unsigned char f_hand)
{
}
void Widget::OnHandDeactivated(unsigned char f_hand)
{
}
void Widget::OnButtonPress(unsigned char f_hand, uint32_t f_button)
{
}
void Widget::OnButtonRelease(unsigned char f_hand, uint32_t f_button)
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

void Widget::SetInterfaces(vr::IVROverlay *f_overlay, vr::IVRCompositor *f_compositor)
{
    ms_vrOverlay = f_overlay;
    ms_vrCompositor = f_compositor;
}
