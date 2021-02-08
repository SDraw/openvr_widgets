#include "stdafx.h"

#include "Widgets/Widget.h"
#include "Utils/VROverlay.h"

#include "Utils/Transformation.h"

Widget::Widget()
{
    m_overlayMain = new VROverlay();
    m_event = { 0 };
    m_size.x = 0.f;
    m_size.y = 0.f;

    m_valid = false;
    m_visible = false;
    m_closed = false;
    m_activeDashboard = false;
}
Widget::~Widget()
{
    delete m_overlayMain;
}

void Widget::Destroy()
{
    m_overlayMain->Destroy();

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
