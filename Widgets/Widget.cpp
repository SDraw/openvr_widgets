#include "stdafx.h"

#include "Widgets/Widget.h"

vr::IVROverlay *Widget::ms_vrOverlay = nullptr;

Widget::Widget()
{
}
Widget::~Widget()
{
}

void Widget::SetInterfaces(vr::IVROverlay *f_overlay)
{
    ms_vrOverlay = f_overlay;
}
