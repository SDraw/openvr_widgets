#include "stdafx.h"

#include "Widgets/Widget.h"
#include "Utils/Transformation.h"

vr::IVROverlay *Widget::ms_vrOverlay = nullptr;

Widget::Widget()
{
    m_valid = false;
    m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    m_vrTexture = { 0 };
    m_visible = false;
    m_transform = new Transformation();
}
Widget::~Widget()
{
    delete m_transform;
}

void Widget::SetInterfaces(vr::IVROverlay *f_overlay)
{
    ms_vrOverlay = f_overlay;
}
