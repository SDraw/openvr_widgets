#include "stdafx.h"

#include "Managers/WidgetManager.h"
#include "Core/Core.h"
#include "Core/VRDevicesStates.h"
#include "Gui/GuiSystem.h"
#include "Gui/GuiElement.h"
#include "Widgets/Widget.h"

#include "Managers/ConfigManager.h"
#include "Gui/GuiStructures.h"
#include "Gui/GuiButton.h"
#include "Widgets/WidgetKeyboard.h"
#include "Widgets/WidgetStats.h"
#include "Widgets/WidgetWindowCapture.h"
#include "Utils/VRDashOverlay.h"

enum ConstantWidgetIndex : size_t
{
    CWI_Stats = 0U,
};

const sf::Vector2f g_buttonSize(320.f, 64.f);
const sf::Vector2u g_guiSize(512U, 512U);

WidgetManager::WidgetManager(Core *f_core)
{
    m_core = f_core;

    // Init with empty fields
    m_dashOverlay = new VRDashOverlay();
    m_event = { 0 };
    m_activeDashboard = false;
    m_guiSystem = nullptr;
    for(size_t i = 0U; i < GEI_Count; i++) m_guiButtons[i] = nullptr;

    // Create settings dashboard overlay
    if(m_dashOverlay->Create("ovrw.settings", "OpenVR Widgets"))
    {
        std::string l_iconPath(ConfigManager::GetDirectory());
        l_iconPath.append("\\icons\\dashboard_icon.png");
        m_dashOverlay->SetThumbTexture(l_iconPath);

        m_dashOverlay->SetInputMethod(vr::VROverlayInputMethod_Mouse);
        m_dashOverlay->SetWidth(1.0f);
        m_dashOverlay->SetMouseScale(512.f, 512.f);
    }

    m_guiSystem = new GuiSystem(g_guiSize);
    if(m_guiSystem->IsValid())
    {
        m_guiSystem->SetFont(ConfigManager::GetGuiFont());

        const auto l_clickCallback = std::bind(&WidgetManager::OnGuiElementMouseClick, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        for(size_t i = 0U; i < GEI_Count; i++)
        {
            m_guiButtons[i] = m_guiSystem->CreateButton();
            m_guiButtons[i]->SetTextSize(20U);
            m_guiButtons[i]->SetSize(g_buttonSize);
            m_guiButtons[i]->SetClickCallback(l_clickCallback);
            m_guiButtons[i]->SetUserPointer(reinterpret_cast<void*>(i));
            m_guiButtons[i]->SetVisibility(false);
        }

        // Main menu
        m_guiButtons[GEI_Widgets]->SetPosition(sf::Vector2f(96.f, 16.f));
        m_guiButtons[GEI_Widgets]->SetText("Widgets >");
        m_guiButtons[GEI_Widgets]->SetVisibility(true);
        m_guiButtons[GEI_Devices]->SetPosition(sf::Vector2f(96.f, 96.f));
        m_guiButtons[GEI_Devices]->SetText("Devices >");
        m_guiButtons[GEI_Devices]->SetVisibility(true);
        m_guiButtons[GEI_Settings]->SetPosition(sf::Vector2f(96.f, 176.f));
        m_guiButtons[GEI_Settings]->SetText("Settings >");
        m_guiButtons[GEI_Settings]->SetVisibility(true);
        m_guiButtons[GEI_Close]->SetPosition(sf::Vector2f(96.f, 256.f));
        m_guiButtons[GEI_Close]->SetText("Close OVR Widgets");
        m_guiButtons[GEI_Close]->SetVisibility(true);

        // Widgets menu
        m_guiButtons[GEI_Widgets_WindowCapture]->SetPosition(sf::Vector2f(96.f, 16.f));
        m_guiButtons[GEI_Widgets_WindowCapture]->SetText("Window capture widget");
        m_guiButtons[GEI_Widgets_Keyboard]->SetPosition(sf::Vector2f(96.f, 96.f));
        m_guiButtons[GEI_Widgets_Keyboard]->SetText("Keyboard widget");
        m_guiButtons[GEI_Widgets_Remove]->SetPosition(sf::Vector2f(96.f, 176.f));
        m_guiButtons[GEI_Widgets_Remove]->SetText("Remove widgets");
        m_guiButtons[GEI_Widgets_Back]->SetPosition(sf::Vector2f(96.f, 256.f));
        m_guiButtons[GEI_Widgets_Back]->SetText("< Back");

        // Devices menu
        m_guiButtons[GEI_Devices_KinectV1]->SetPosition(sf::Vector2f(96.f, 16.f));
        m_guiButtons[GEI_Devices_KinectV1]->SetText("Switch KinectV1");
        m_guiButtons[GEI_Devices_KinectV2]->SetPosition(sf::Vector2f(96.f, 96.f));
        m_guiButtons[GEI_Devices_KinectV2]->SetText("Switch KinectV2");
        m_guiButtons[GEI_Devices_LMotionLeft]->SetPosition(sf::Vector2f(96.f, 176.f));
        m_guiButtons[GEI_Devices_LMotionLeft]->SetText("Switch LMotion left hand");
        m_guiButtons[GEI_Devices_LMotionRight]->SetPosition(sf::Vector2f(96.f, 256.f));
        m_guiButtons[GEI_Devices_LMotionRight]->SetText("Switch LMotion right hand");
        m_guiButtons[GEI_Devices_Back]->SetPosition(sf::Vector2f(96.f, 336.f));
        m_guiButtons[GEI_Devices_Back]->SetText("< Back");

        // Settings menu
        m_guiButtons[GEI_Settings_FPS]->SetPosition(sf::Vector2f(96.f, 16.f));
        switch(m_core->GetConfigManager()->GetTargetRate())
        {
            case ConfigManager::TF_60:
                m_guiButtons[GEI_Settings_FPS]->SetText("Target FPS: 60");
                break;
            case ConfigManager::TF_90:
                m_guiButtons[GEI_Settings_FPS]->SetText("Target FPS: 90");
                break;
            case ConfigManager::TF_120:
                m_guiButtons[GEI_Settings_FPS]->SetText("Target FPS: 120");
                break;
            case ConfigManager::TF_144:
                m_guiButtons[GEI_Settings_FPS]->SetText("Target FPS: 144");
                break;
        }
        m_guiButtons[GEI_Settings_ReassignHands]->SetPosition(sf::Vector2f(96.f, 96.f));
        m_guiButtons[GEI_Settings_ReassignHands]->SetText("Reassign hands");
        m_guiButtons[GEI_Settings_Back]->SetPosition(sf::Vector2f(96.f, 176.f));
        m_guiButtons[GEI_Settings_Back]->SetText("< Back");

        m_dashOverlay->SetTexture(m_guiSystem->GetRenderTextureHandle());
    }

    // Init static widget resources
    WidgetKeyboard::InitStaticResources();
    WidgetWindowCapture::InitStaticResources();

    // Init constant overlays
    m_constantWidgets.emplace(ConstantWidgetIndex::CWI_Stats, new WidgetStats());
    for(auto l_iter : m_constantWidgets) l_iter.second->Create();

    m_activeDashboard = vr::VROverlay()->IsDashboardVisible();
    if(m_activeDashboard) OnDashboardOpen();
}
WidgetManager::~WidgetManager()
{
    m_dashOverlay->Destroy();
    delete m_dashOverlay;

    delete m_guiSystem;

    // Destroy active widgets
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

    WidgetKeyboard::RemoveStaticResources();
    WidgetWindowCapture::RemoveStaticResources();
}

void WidgetManager::DoPulse()
{
    if(m_activeDashboard && m_dashOverlay->IsVisible())
    {
        while(m_dashOverlay->Poll(m_event))
        {
            switch(m_event.eventType)
            {
                case vr::VREvent_MouseButtonDown: case vr::VREvent_MouseButtonUp:
                {
                    unsigned char l_button = 0U;
                    switch(m_event.data.mouse.button)
                    {
                        case vr::VRMouseButton_Left:
                            l_button = GuiClick::GC_Left;
                            break;
                        case vr::VRMouseButton_Right:
                            l_button = GuiClick::GC_Right;
                            break;
                        case vr::VRMouseButton_Middle:
                            l_button = GuiClick::GC_Middle;
                            break;
                    }
                    const unsigned char l_state = ((m_event.eventType == vr::VREvent_MouseButtonDown) ? GuiClickState::GCS_Press : GuiClickState::GCS_Release);
#ifdef _WIN32
                    m_guiSystem->ProcessClick(l_button, l_state, static_cast<unsigned int>(m_event.data.mouse.x), static_cast<unsigned int>(m_event.data.mouse.y));
#elif __linux__
                    m_guiSystem->ProcessClick(l_button, l_state, static_cast<unsigned int>(m_event.data.mouse.x), static_cast<unsigned int>(g_guiSize.y - m_event.data.mouse.y));
#endif
                } break;
                case vr::VREvent_MouseMove:
#ifdef _WIN32
                    m_guiSystem->ProcessMove(static_cast<unsigned int>(m_event.data.mouse.x), static_cast<unsigned int>(m_event.data.mouse.y));
#elif __linux__
                    m_guiSystem->ProcessMove(static_cast<unsigned int>(m_event.data.mouse.x), static_cast<unsigned int>(g_guiSize.y - m_event.data.mouse.y));
#endif
                    break;
            }
        }

        m_guiSystem->Update();
        m_dashOverlay->Update();
    }

    for(auto l_iter : m_constantWidgets) l_iter.second->Update();
    for(auto l_iter = m_widgets.begin(); l_iter != m_widgets.end();)
    {
        Widget *l_widget = (*l_iter);
        l_widget->Update();
        if(l_widget->IsClosed())
        {
            l_widget->Destroy();
            delete l_widget;

            l_iter = m_widgets.erase(l_iter);
        }
        else ++l_iter;
    }
}

void WidgetManager::OnHandActivated(size_t f_hand)
{
    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnHandActivated(f_hand);
    for(auto l_widget : m_widgets) l_widget->OnHandActivated(f_hand);
}
void WidgetManager::OnHandDeactivated(size_t f_hand)
{
    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnHandDeactivated(f_hand);
    for(auto l_widget : m_widgets) l_widget->OnHandDeactivated(f_hand);
}

void WidgetManager::OnButtonPress(size_t f_hand, uint32_t f_button)
{
    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnButtonPress(f_hand, f_button);
    for(auto l_widget : m_widgets) l_widget->OnButtonPress(f_hand, f_button);
}
void WidgetManager::OnButtonRelease(size_t f_hand, uint32_t f_button)
{
    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnButtonRelease(f_hand, f_button);
    for(auto l_widget : m_widgets) l_widget->OnButtonRelease(f_hand, f_button);
}

void WidgetManager::OnDashboardOpen()
{
    m_activeDashboard = true;

    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnDashboardOpen();
    for(auto l_widget : m_widgets) l_widget->OnDashboardOpen();
}
void WidgetManager::OnDashboardClose()
{
    m_activeDashboard = false;

    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnDashboardClose();
    for(auto l_widget : m_widgets) l_widget->OnDashboardClose();
}

void WidgetManager::OnGuiElementMouseClick(GuiElement *f_guiElement, unsigned char f_button, unsigned char f_state)
{
    if((f_button == GuiClick::GC_Left) && (f_state == GuiClickState::GCS_Press))
    {
        switch(reinterpret_cast<size_t>(f_guiElement->GetUserPointer())) // Bold move for someone within stabbing range
        {
            // Main menu
            case GEI_Widgets:
            {
                m_guiButtons[GEI_Widgets]->SetVisibility(false);
                m_guiButtons[GEI_Devices]->SetVisibility(false);
                m_guiButtons[GEI_Settings]->SetVisibility(false);
                m_guiButtons[GEI_Close]->SetVisibility(false);
                for(size_t i = GEI_Widgets_WindowCapture; i <= GEI_Widgets_Back; i++) m_guiButtons[i]->SetVisibility(true);
            } break;
            case GEI_Devices:
            {
                m_guiButtons[GEI_Widgets]->SetVisibility(false);
                m_guiButtons[GEI_Devices]->SetVisibility(false);
                m_guiButtons[GEI_Settings]->SetVisibility(false);
                m_guiButtons[GEI_Close]->SetVisibility(false);
                for(size_t i = GEI_Devices_KinectV1; i <= GEI_Devices_Back; i++) m_guiButtons[i]->SetVisibility(true);
            } break;
            case GEI_Settings:
            {
                m_guiButtons[GEI_Widgets]->SetVisibility(false);
                m_guiButtons[GEI_Devices]->SetVisibility(false);
                m_guiButtons[GEI_Settings]->SetVisibility(false);
                m_guiButtons[GEI_Close]->SetVisibility(false);
                for(size_t i = GEI_Settings_FPS; i <= GEI_Settings_Back; i++) m_guiButtons[i]->SetVisibility(true);
            } break;
            case GEI_Close:
                m_core->RequestExit();
                break;

            // Widgets menu
            case GEI_Widgets_WindowCapture:
            {
                Widget *l_widget = new WidgetWindowCapture();
                if(l_widget->Create())
                {
                    if(m_activeDashboard) l_widget->OnDashboardOpen();
                    m_widgets.push_back(l_widget);
                }
                else
                {
                    l_widget->Destroy();
                    delete l_widget;
                }
            } break;
            case GEI_Widgets_Keyboard:
            {
                Widget *l_widget = new WidgetKeyboard();
                if(l_widget->Create())
                {
                    if(m_activeDashboard) l_widget->OnDashboardOpen();
                    m_widgets.push_back(l_widget);
                }
                else
                {
                    l_widget->Destroy();
                    delete l_widget;
                }
            } break;
            case GEI_Widgets_Remove:
            {
                for(auto l_widget : m_widgets)
                {
                    l_widget->Destroy();
                    delete l_widget;
                }
                m_widgets.clear();
            } break;
            case GEI_Widgets_Back:
            {
                for(size_t i = GEI_Widgets_WindowCapture; i <= GEI_Widgets_Back; i++) m_guiButtons[i]->SetVisibility(false);
                m_guiButtons[GEI_Widgets]->SetVisibility(true);
                m_guiButtons[GEI_Devices]->SetVisibility(true);
                m_guiButtons[GEI_Settings]->SetVisibility(true);
                m_guiButtons[GEI_Close]->SetVisibility(true);
            } break;

            case GEI_Devices_KinectV1:
                m_core->SendMessageToDeviceWithProperty(0x4B696E6563745631, "switch"); // Refer to driver_kinectV1
                break;
            case GEI_Devices_KinectV2:
                m_core->SendMessageToDeviceWithProperty(0x4B696E6563745632, "switch"); // Refer to driver_kinectV2
                break;
            case GEI_Devices_LMotionLeft:
                m_core->SendMessageToDeviceWithProperty(0x4C4D6F74696F6E, "setting left_hand"); // Refer to driver_leap
                break;
            case GEI_Devices_LMotionRight:
                m_core->SendMessageToDeviceWithProperty(0x4C4D6F74696F6E, "setting left_hand"); // Refer to driver_leap
                break;
            case GEI_Devices_Back:
            {
                for(size_t i = GEI_Devices_KinectV1; i <= GEI_Devices_Back; i++) m_guiButtons[i]->SetVisibility(false);
                m_guiButtons[GEI_Widgets]->SetVisibility(true);
                m_guiButtons[GEI_Devices]->SetVisibility(true);
                m_guiButtons[GEI_Settings]->SetVisibility(true);
                m_guiButtons[GEI_Close]->SetVisibility(true);
            } break;

            // Settings menu
            case GEI_Settings_FPS:
            {
                m_core->GetConfigManager()->SetTargetRate(m_core->GetConfigManager()->GetTargetRate() + 1U);
                switch(m_core->GetConfigManager()->GetTargetRate())
                {
                    case ConfigManager::TF_60:
                        m_guiButtons[GEI_Settings_FPS]->SetText("Target FPS: 60");
                        break;
                    case ConfigManager::TF_90:
                        m_guiButtons[GEI_Settings_FPS]->SetText("Target FPS: 90");
                        break;
                    case ConfigManager::TF_120:
                        m_guiButtons[GEI_Settings_FPS]->SetText("Target FPS: 120");
                        break;
                    case ConfigManager::TF_144:
                        m_guiButtons[GEI_Settings_FPS]->SetText("Target FPS: 144");
                        break;
                }
                m_core->UpdateTargetRate();
            } break;
            case GEI_Settings_ReassignHands:
                m_core->ForceHandSearch();
                break;
            case GEI_Settings_Back:
            {
                for(size_t i = GEI_Settings_FPS; i <= GEI_Settings_Back; i++) m_guiButtons[i]->SetVisibility(false);
                m_guiButtons[GEI_Widgets]->SetVisibility(true);
                m_guiButtons[GEI_Devices]->SetVisibility(true);
                m_guiButtons[GEI_Settings]->SetVisibility(true);
                m_guiButtons[GEI_Close]->SetVisibility(true);
            } break;
        }
    }
}
