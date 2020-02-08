#include "stdafx.h"

#include "Widgets/Widget.h"
#include "Utils/Transformation.h"

#include "Utils/GlobalStructures.h"

vr::IVROverlay *Widget::ms_vrOverlay = nullptr;
vr::IVRCompositor *Widget::ms_vrCompositor = nullptr;

Widget::Widget()
{
    m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    m_vrTexture = { 0 };

    m_valid = false;
    m_visible = false;

    m_language = Language::Language_English;
    m_transform = new Transformation();
}
Widget::~Widget()
{
    delete m_transform;
}

void Widget::OnLanguageChange(unsigned char f_lang)
{
    if(m_valid) m_language = f_lang;
}

void Widget::SetInterfaces(vr::IVROverlay *f_overlay, vr::IVRCompositor *f_compositor)
{
    ms_vrOverlay = f_overlay;
    ms_vrCompositor = f_compositor;
}
