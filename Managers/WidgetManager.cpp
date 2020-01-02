#include "stdafx.h"

#include "Managers/WidgetManager.h"
#include "Core/Core.h"
#include "Gui/GuiSystem.h"
#include "Gui/GuiElement.h"
#include "Widgets/Widget.h"

#include "Managers/ConfigManager.h"
#include "Gui/GuiStructures.h"
#include "Gui/GuiButton.h"
#include "Widgets/WidgetStats.h"
#include "Widgets/WidgetWindowCapture.h"
#include "Utils/GlobalStructures.h"

const char* g_ButtonsTextsEn[] = {
    "Add window capture widget"
};
const wchar_t* g_ButtonsTextsRu[] = {
    L"\u0414\u043E\u0431\u0430\u0432\u0438\u0442\u044C \u0432\u0438\u0434\u0436\u0435\u0442 \u0437\u0430\u0445\u0432\u0430\u0442\u0430 \u043E\u043A\u043D\u0430"
};

enum ConstantWidget : size_t
{
    ConstantWidget_Stats = 0U
};

const sf::Color g_HoverColor(142U, 205U, 246U);

WidgetManager::WidgetManager(Core *f_core)
{
    m_core = f_core;

    // Init with empty fields
    m_guiSystem = nullptr;
    m_overlayDashboardThumbnail = vr::k_ulOverlayHandleInvalid;
    m_overlayDashboard = vr::k_ulOverlayHandleInvalid;
    m_textureDashboard = { 0 };
    m_overlayEvent = { 0 };
    m_activeDashboard = false;

    // Create settings dashboard overlay
    sf::Vector2u l_guiSize(512U, 512U);
    m_guiSystem = new GuiSystem(l_guiSize);
    for(size_t i = 0U; i < GuiElementIndex_Max; i++) m_guiElements[i] = nullptr;
    if(m_guiSystem->IsValid())
    {
        m_guiSystem->SetButtonsTexture(m_core->GetConfigManager()->GetGuiButton());
        m_guiSystem->SetFont(m_core->GetConfigManager()->GetGuiFont());

        std::function<void(void*, unsigned char, unsigned char, unsigned int, unsigned int)> l_clickCallback([this](void *f_guiElement, unsigned char f_button, unsigned char f_state, unsigned int, unsigned int)
        {
            this->OnGuiElementMouseClick(f_guiElement, f_button, f_state);
        });

        m_guiElements[GuiElementIndex_AddCaptureWindow] = m_guiSystem->CreateButton();
        m_guiElements[GuiElementIndex_AddCaptureWindow]->SetPosition(sf::Vector2f(96.f, 16.f));
        m_guiElements[GuiElementIndex_AddCaptureWindow]->SetSize(sf::Vector2f(320.f, 64.f));
        m_guiElements[GuiElementIndex_AddCaptureWindow]->SetMouseClickCallback(l_clickCallback);
        m_guiElements[GuiElementIndex_AddCaptureWindow]->SetHoverColor(g_HoverColor);
        dynamic_cast<GuiButton*>(m_guiElements[GuiElementIndex_AddCaptureWindow])->SetTextSize(20U);

        switch(m_core->GetConfigManager()->GetLanguage())
        {
            case Language::Language_English:
            {
                dynamic_cast<GuiButton*>(m_guiElements[GuiElementIndex_AddCaptureWindow])->SetText(g_ButtonsTextsEn[GuiElementIndex_AddCaptureWindow]);
            } break;
            case Language::Language_Russian:
            {
                dynamic_cast<GuiButton*>(m_guiElements[GuiElementIndex_AddCaptureWindow])->SetText(g_ButtonsTextsRu[GuiElementIndex_AddCaptureWindow]);
            } break;
        }
    }

    m_core->GetVROverlay()->CreateDashboardOverlay("ovrw.settings", "OpenVR Widgets\nSettings", &m_overlayDashboard, &m_overlayDashboardThumbnail);
    if((m_overlayDashboard != vr::k_ulOverlayHandleInvalid) && (m_overlayDashboardThumbnail != vr::k_ulOverlayHandleInvalid))
    {
        std::string l_iconPath(m_core->GetConfigManager()->GetDirectory());
        l_iconPath.append("\\icons\\dashboard_icon.png");
        m_core->GetVROverlay()->SetOverlayFromFile(m_overlayDashboardThumbnail, l_iconPath.c_str());

        m_core->GetVROverlay()->SetOverlayInputMethod(m_overlayDashboard, vr::VROverlayInputMethod_Mouse);
        m_core->GetVROverlay()->SetOverlayWidthInMeters(m_overlayDashboard, 1.0f);

        vr::HmdVector2_t l_mouseScale = { 512.f, 512.f };
        m_core->GetVROverlay()->SetOverlayMouseScale(m_overlayDashboard, &l_mouseScale);

        m_textureDashboard.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(m_guiSystem->GetRenderTextureHandle()));
        m_textureDashboard.eType = vr::TextureType_OpenGL;
        m_textureDashboard.eColorSpace = vr::ColorSpace_Gamma;
        m_core->GetVROverlay()->SetOverlayTexture(m_overlayDashboard, &m_textureDashboard);
    }

    // Init constant overlays
    Widget::SetInterfaces(m_core->GetVROverlay(), m_core->GetVRCompositor());
    m_constantWidgets.emplace(ConstantWidget_Stats, new WidgetStats());
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

    WidgetWindowCapture::RemoveIconsAtlas();
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
                                l_button = GuiMouseClick::GuiMouseClick_Left;
                                break;
                            case vr::VRMouseButton_Right:
                                l_button = GuiMouseClick::GuiMouseClick_Right;
                                break;
                            case vr::VRMouseButton_Middle:
                                l_button = GuiMouseClick::GuiMouseClick_Middle;
                                break;
                        }
                        unsigned char l_state = ((m_overlayEvent.eventType == vr::VREvent_MouseButtonDown) ? GuiMouseClickState::GuiClickState_Press : GuiMouseClickState::GuiClickState_Release);
                        m_guiSystem->ProcessMouseClick(l_button, l_state, static_cast<unsigned int>(m_overlayEvent.data.mouse.x), static_cast<unsigned int>(m_overlayEvent.data.mouse.y));
                    } break;
                    case vr::VREvent_MouseMove:
                        m_guiSystem->ProcessMouseMove(static_cast<unsigned int>(m_overlayEvent.data.mouse.x), static_cast<unsigned int>(m_overlayEvent.data.mouse.y));
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
        if(l_widget->CloseRequested())
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

void WidgetManager::OnGuiElementMouseClick(void *f_guiElement, unsigned char f_button, unsigned char f_state)
{
    size_t l_index = std::numeric_limits<size_t>::max();
    for(size_t i = 0U; i < GuiElementIndex::GuiElementIndex_Max; i++)
    {
        if(m_guiElements[i] == f_guiElement)
        {
            l_index = i;
            break;
        }
    }
    if(l_index != std::numeric_limits<size_t>::max())
    {
        switch(l_index)
        {
            case GuiElementIndex::GuiElementIndex_AddCaptureWindow:
            {
                if((f_button == GuiMouseClick::GuiMouseClick_Left) && (f_state == GuiMouseClickState::GuiClickState_Press))
                {
                    Widget *l_widget = new WidgetWindowCapture();
                    if(l_widget->Create())
                    {
                        l_widget->OnLanguageChange(m_core->GetConfigManager()->GetLanguage());
                        if(m_activeDashboard) l_widget->OnDashboardOpen();
                        m_widgets.push_back(l_widget);
                    }
                    else delete l_widget;
                }
            } break;
        }
    }
}