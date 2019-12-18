#include "stdafx.h"

#include "Managers/WidgetManager.h"
#include "Core/Core.h"
#include "Widgets/Widget.h"

#include "Managers/ConfigManager.h"
#include "Widgets/WidgetStats.h"
#include "Widgets/WidgetWindowCapture.h"

enum EConstantWidget : unsigned char
{
    EConstantWidget_Stats = 0U,
    EConstantWidget_WindowCapture
};

WidgetManager::WidgetManager(Core *f_core)
{
    m_core = f_core;

    // Create settings overlays at right side of player

    // Init constant overlays
    Widget::SetInterfaces(m_core->GetVROverlay(), m_core->GetVRCompositor());
    m_constantWidgets.emplace(EConstantWidget::EConstantWidget_Stats, new WidgetStats());
    m_constantWidgets.emplace(EConstantWidget::EConstantWidget_WindowCapture, new WidgetWindowCapture());
    for(auto l_iter : m_constantWidgets)
    {
        if(l_iter.second->Create())
        {
            l_iter.second->OnLanguageChange(m_core->GetConfigManager()->GetLanguage());
        }
    }
}
WidgetManager::~WidgetManager()
{
    // Save settings?

    // Destroy active overlays
    for(auto l_iter : m_constantWidgets)
    {
        l_iter.second->Destroy();
        delete l_iter.second;
    }
    m_constantWidgets.clear();

    for(auto l_widget : m_widgets)
    {
        l_widget->Destroy();
        delete l_widget;
    }
    m_widgets.clear();

    Widget::SetInterfaces(nullptr, nullptr);
    // Destroy settings overlays

}

void WidgetManager::DoPulse()
{
    for(auto l_iter : m_constantWidgets) l_iter.second->Update();
    for(auto l_widget : m_widgets) l_widget->Update();
}

void WidgetManager::OnHandActivated(unsigned char f_hand)
{
    // Do own stuff

    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnHandActivated(f_hand);
    for(auto l_widget : m_widgets) l_widget->OnHandActivated(f_hand);
}
void WidgetManager::OnHandDeactivated(unsigned char f_hand)
{
    // Do own stuff

    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnHandDeactivated(f_hand);
    for(auto l_widget : m_widgets) l_widget->OnHandDeactivated(f_hand);
}
void WidgetManager::OnButtonPress(unsigned char f_hand, uint32_t f_button)
{
    // Do own stuff

    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnButtonPress(f_hand, f_button);
    for(auto l_widget : m_widgets) l_widget->OnButtonPress(f_hand, f_button);
}
void WidgetManager::OnButtonRelease(unsigned char f_hand, uint32_t f_button)
{
    // Do own stuff

    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnButtonRelease(f_hand, f_button);
    for(auto l_widget : m_widgets) l_widget->OnButtonRelease(f_hand, f_button);
}
void WidgetManager::OnDashboardOpen()
{
    // Do own stuff

    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnDashboardOpen();
    for(auto l_widget : m_widgets) l_widget->OnDashboardOpen();
}
void WidgetManager::OnDashboardClose()
{
    // Do own stuff

    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnDashboardClose();
    for(auto l_widget : m_widgets) l_widget->OnDashboardClose();
}