#include "stdafx.h"

#include "Managers/WidgetManager.h"
#include "Core/Core.h"
#include "Gui/GuiSystem.h"
#include "Gui/GuiElement.h"
#include "Widgets/Widget.h"

#include "Managers/ConfigManager.h"
#include "Gui/GuiStructures.h"
#include "Gui/GuiButton.h"
#include "Widgets/WidgetKeyboard.h"
#include "Widgets/WidgetStats.h"
#include "Widgets/WidgetWindowCapture.h"
#include "Utils/GlobalStructures.h"

#include "Core/VRTransform.h"

const char *g_ButtonsTextsEn[]
{
    "Add window capture widget",
    "Add keyboard widget",
    "Reassign hands",
    "Switch KinectV2 tracking",
    "Switch Leap left hand",
    "Switch Leap right hand"
};
const wchar_t *g_ButtonsTextsRu[]
{
    L"\u0414\u043E\u0431\u0430\u0432\u0438\u0442\u044C \u0432\u0438\u0434\u0436\u0435\u0442 \u0437\u0430\u0445\u0432\u0430\u0442\u0430",
    L"\u0414\u043E\u0431\u0430\u0432\u0438\u0442\u044C \u043A\u043B\u0430\u0432\u0438\u0430\u0442\u0443\u0440\u0443",
    L"\u041E\u043F\u0440\u0435\u0434\u0435\u043B\u0438\u0442\u044C \u0440\u0443\u043A\u0438",
    L"\u0418\u0437\u043C\u0435\u043D\u0438\u0442\u044C \u043E\u0442\u0441\u043B\u0435\u0436\u0438\u0432\u0430\u043D\u0438\u0435 KinectV2",
    L"\u0418\u0437\u043C\u0435\u043D\u0438\u0442\u044C \u043E\u0442\u0441\u043B\u0435\u0436\u0438\u0432\u0430\u043D\u0438\u0435 LM \u043B\u0435\u0432\u043E\u0439 \u0440\u0443\u043A\u0438",
    L"\u0418\u0437\u043C\u0435\u043D\u0438\u0442\u044C \u043E\u0442\u0441\u043B\u0435\u0436\u0438\u0432\u0430\u043D\u0438\u0435 LM \u043F\u0440\u0430\u0432\u043E\u0439 \u0440\u0443\u043A\u0438"
};

enum ConstantWidgetIndex : size_t
{
    CWI_Stats = 0U
};

const sf::Vector2f g_ButtonSize(320.f, 64.f);
const sf::Vector2u g_GuiSize(512U, 512U);

WidgetManager::WidgetManager(Core *f_core)
{
    m_core = f_core;

    // Init with empty fields
    m_overlayDashboardThumbnail = vr::k_ulOverlayHandleInvalid;
    m_overlayDashboard = vr::k_ulOverlayHandleInvalid;
    m_textureDashboard = { 0 };
    m_overlayEvent = { 0 };
    m_activeDashboard = false;
    m_guiSystem = nullptr;
    for(size_t i = 0U; i < GEI_Max; i++) m_guiElements[i] = nullptr;

    // Create settings dashboard overlay
    m_guiSystem = new GuiSystem(g_GuiSize);
    if(m_guiSystem->IsValid())
    {
        m_guiSystem->SetFont(m_core->GetConfigManager()->GetGuiFont());

        const std::function<void(GuiElement*, unsigned char, unsigned char, unsigned int, unsigned int)> l_clickCallback([this](GuiElement *f_guiElement, unsigned char f_button, unsigned char f_state, unsigned int, unsigned int)
        {
            this->OnGuiElementMouseClick(f_guiElement, f_button, f_state);
        });

        for(size_t i = 0U; i < GEI_Max; i++)
        {
            m_guiElements[i] = m_guiSystem->CreateButton();
            m_guiElements[i]->SetPosition(sf::Vector2f(96.f, 16.f + 80.f * static_cast<float>(i)));
            m_guiElements[i]->SetSize(g_ButtonSize);
            dynamic_cast<GuiButton*>(m_guiElements[i])->SetTextSize(20U);

            switch(m_core->GetConfigManager()->GetLanguage())
            {
                case LanguageIndex::LI_English:
                    dynamic_cast<GuiButton*>(m_guiElements[i])->SetText(g_ButtonsTextsEn[i]);
                    break;
                case LanguageIndex::LI_Russian:
                    dynamic_cast<GuiButton*>(m_guiElements[i])->SetText(g_ButtonsTextsRu[i]);
                    break;
            }

            m_guiElements[i]->SetClickCallback(l_clickCallback);
            m_guiElements[i]->SetUserPointer(reinterpret_cast<void*>(i));
        }
    }

    m_core->GetVROverlay()->CreateDashboardOverlay("ovrw.settings", "OpenVR Widgets Settings", &m_overlayDashboard, &m_overlayDashboardThumbnail);
    if((m_overlayDashboard != vr::k_ulOverlayHandleInvalid) && (m_overlayDashboardThumbnail != vr::k_ulOverlayHandleInvalid))
    {
        std::string l_iconPath(m_core->GetConfigManager()->GetDirectory());
        l_iconPath.append("\\icons\\dashboard_icon.png");
        m_core->GetVROverlay()->SetOverlayFromFile(m_overlayDashboardThumbnail, l_iconPath.c_str());

        m_core->GetVROverlay()->SetOverlayInputMethod(m_overlayDashboard, vr::VROverlayInputMethod_Mouse);
        m_core->GetVROverlay()->SetOverlayWidthInMeters(m_overlayDashboard, 1.0f);

        const vr::HmdVector2_t l_mouseScale = { 512.f, 512.f };
        m_core->GetVROverlay()->SetOverlayMouseScale(m_overlayDashboard, &l_mouseScale);

        m_textureDashboard.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(m_guiSystem->GetRenderTextureHandle()));
        m_textureDashboard.eType = vr::TextureType_OpenGL;
        m_textureDashboard.eColorSpace = vr::ColorSpace_Gamma;
        m_core->GetVROverlay()->SetOverlayTexture(m_overlayDashboard, &m_textureDashboard);
    }

    // Init constant overlays
    Widget::SetInterfaces(m_core->GetVROverlay(), m_core->GetVRCompositor());
    m_constantWidgets.emplace(ConstantWidgetIndex::CWI_Stats, new WidgetStats());
    for(auto l_iter : m_constantWidgets)
    {
        if(l_iter.second->Create())
        {
            l_iter.second->OnLanguageChange(m_core->GetConfigManager()->GetLanguage());
        }
    }

    m_activeDashboard = m_core->GetVROverlay()->IsDashboardVisible();
    if(m_activeDashboard) OnDashboardOpen();
}
WidgetManager::~WidgetManager()
{
    // Save settings?
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

    WidgetWindowCapture::RemoveStaticResources();
    WidgetKeyboard::RemoveStaticResources();
    Widget::SetInterfaces(nullptr, nullptr);
}

void WidgetManager::DoPulse()
{
    if(m_overlayDashboard != vr::k_ulOverlayHandleInvalid)
    {
        if(m_activeDashboard && m_core->GetVROverlay()->IsOverlayVisible(m_overlayDashboard))
        {
            while(m_core->GetVROverlay()->PollNextOverlayEvent(m_overlayDashboard, &m_overlayEvent, sizeof(vr::VREvent_t)))
            {
                switch(m_overlayEvent.eventType)
                {
                    case vr::VREvent_MouseButtonDown: case vr::VREvent_MouseButtonUp:
                    {
                        unsigned char l_button = 0U;
                        switch(m_overlayEvent.data.mouse.button)
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
                        const unsigned char l_state = ((m_overlayEvent.eventType == vr::VREvent_MouseButtonDown) ? GuiClickState::GCS_Press : GuiClickState::GCS_Release);
                        m_guiSystem->ProcessClick(l_button, l_state, static_cast<unsigned int>(m_overlayEvent.data.mouse.x), static_cast<unsigned int>(m_overlayEvent.data.mouse.y));
                    } break;
                    case vr::VREvent_MouseMove:
                        m_guiSystem->ProcessMove(static_cast<unsigned int>(m_overlayEvent.data.mouse.x), static_cast<unsigned int>(m_overlayEvent.data.mouse.y));
                        break;
                }
            }

            m_guiSystem->Update();
            m_core->GetVROverlay()->SetOverlayTexture(m_overlayDashboard, &m_textureDashboard);
        }
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

void WidgetManager::OnHandActivated(unsigned char f_hand)
{
    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnHandActivated(f_hand);
    for(auto l_widget : m_widgets) l_widget->OnHandActivated(f_hand);
}
void WidgetManager::OnHandDeactivated(unsigned char f_hand)
{
    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnHandDeactivated(f_hand);
    for(auto l_widget : m_widgets) l_widget->OnHandDeactivated(f_hand);
}

void WidgetManager::OnButtonPress(unsigned char f_hand, uint32_t f_button)
{
    // Update widgets
    for(auto l_iter : m_constantWidgets) l_iter.second->OnButtonPress(f_hand, f_button);
    for(auto l_widget : m_widgets) l_widget->OnButtonPress(f_hand, f_button);
}
void WidgetManager::OnButtonRelease(unsigned char f_hand, uint32_t f_button)
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
            case GuiElementIndex::GEI_AddWindowCapture:
            {
                Widget *l_widget = new WidgetWindowCapture();
                if(l_widget->Create())
                {
                    l_widget->OnLanguageChange(m_core->GetConfigManager()->GetLanguage());
                    if(m_activeDashboard) l_widget->OnDashboardOpen();
                    m_widgets.push_back(l_widget);
                }
                else
                {
                    l_widget->Destroy();
                    delete l_widget;
                }
            } break;
            case GEI_AddKeyboard:
            {
                Widget *l_widget = new WidgetKeyboard();
                if(l_widget->Create())
                {
                    l_widget->OnLanguageChange(m_core->GetConfigManager()->GetLanguage());
                    if(m_activeDashboard) l_widget->OnDashboardOpen();
                    m_widgets.push_back(l_widget);
                }
                else
                {
                    l_widget->Destroy();
                    delete l_widget;
                }
            } break;
            case GEI_ReassignHands:
                m_core->ForceHandSearch();
                break;
            case GEI_SwitchKinectTracking:
                m_core->SendMessageToDeviceWithProperty(0x4B696E6563745632, "switch"); // Refer to driver_kinectV2
                break;
            case GEI_SwitchLeapLeftHand:
                m_core->SendMessageToDeviceWithProperty(0x4C4D6F74696F6E, "setting left_hand"); // Refer to driver_leap - leap_monitor
                break;
            case GEI_SwitchLeapRightHand:
                m_core->SendMessageToDeviceWithProperty(0x4C4D6F74696F6E, "setting right_hand"); // Refer to driver_leap - leap_monitor
                break;
        }
    }
}
