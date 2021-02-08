#include "stdafx.h"

#include "Widgets/WidgetKeyboard.h"
#include "Gui/GuiSystem.h"
#include "Gui/GuiButton.h"
#include "Utils/Transformation.h"

#include "Core/VRDevicesStates.h"
#include "Managers/ConfigManager.h"
#include "Gui/GuiStructures.h"
#include "Utils/VROverlay.h"
#include "Utils/Utils.h"

extern const float g_piHalf;
extern const glm::vec3 g_axisX;
extern const glm::vec3 g_axisZN;
extern const unsigned char g_dummyTextureData[];

const sf::Vector2u g_targetSize(1594U, 450U);
const sf::Color g_hoverColor(142U, 205U, 246U);
const std::string g_layoutStateNames[]
{
    "Main layout", "Alternative layout"
};

sf::Texture *WidgetKeyboard::ms_textureAtlas = nullptr;

WidgetKeyboard::WidgetKeyboard()
{
    m_size.x = 0.5f;
    m_size.y = 0.25f;

    m_guiSystem = nullptr;
    m_layoutState = LS_Main;
    m_activeMove = false;
    m_activePin = false;
    m_lastTriggerTick = 0U;
}
WidgetKeyboard::~WidgetKeyboard()
{
}

bool WidgetKeyboard::Create()
{
    if(!m_valid)
    {
        std::string l_overlayKeyPart("ovrw.keyboard_");
        l_overlayKeyPart.append(std::to_string(reinterpret_cast<size_t>(this)));

        std::string l_overlayKeyFull(l_overlayKeyPart);
        l_overlayKeyFull.append(".main");
        if(m_overlayMain->Create(l_overlayKeyFull.c_str(), "OpenVR Widgets - Keyboard - Main"))
        {
            // Create overlay in front of user
            glm::vec3 l_hmdPos;
            glm::quat l_hmdRot;
            VRDevicesStates::GetDevicePosition(VRDeviceIndex::VDI_Hmd, l_hmdPos);
            VRDevicesStates::GetDeviceRotation(VRDeviceIndex::VDI_Hmd, l_hmdRot);
            glm::vec3 l_pos = l_hmdPos + (l_hmdRot*g_axisZN)*0.5f;
            m_overlayMain->GetTransform()->SetPosition(l_pos);

            glm::quat l_rot;
            GetRotationToPoint(l_hmdPos, l_pos, l_hmdRot, l_rot);
            m_overlayMain->GetTransform()->SetRotation(l_rot);

            m_overlayMain->SetInputMethod(vr::VROverlayInputMethod_Mouse);
            m_overlayMain->SetMouseScale(static_cast<float>(g_targetSize.x), static_cast<float>(g_targetSize.y));
            m_overlayMain->SetWidth(m_size.x);
        }

        m_guiSystem = new GuiSystem(g_targetSize);
        if(m_guiSystem->IsValid())
        {
            m_guiSystem->SetFont(ConfigManager::GetGuiFont());

            const auto l_clickCallback = std::bind(&WidgetKeyboard::OnGuiElementClick_Keys, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

            pugi::xml_document *l_layout = new pugi::xml_document();
            if(l_layout->load_file(ConfigManager::GetKeyboardLayout().c_str()))
            {
                pugi::xml_node l_root = l_layout->child("layout");
                if(l_root)
                {
                    for(pugi::xml_node l_keyNode = l_root.child("key"); l_keyNode; l_keyNode = l_keyNode.next_sibling("key"))
                    {
                        pugi::xml_attribute l_attribText = l_keyNode.attribute("text");
                        pugi::xml_attribute l_attribAltText = l_keyNode.attribute("altText");
                        pugi::xml_attribute l_attribCode = l_keyNode.attribute("code");
                        pugi::xml_attribute l_attribTransform = l_keyNode.attribute("transform");
                        if(l_attribText && l_attribAltText && l_attribCode && l_attribTransform)
                        {
                            KeyboadKey *l_keyboardKey = new KeyboadKey();
                            l_keyboardKey->m_text.assign(l_attribText.as_string("?"));
                            l_keyboardKey->m_altText.assign(l_attribAltText.as_string());
                            l_keyboardKey->m_code = l_attribCode.as_uint(0U);

                            GuiButton *l_guiButton = m_guiSystem->CreateButton();
                            l_guiButton->SetUserPointer(l_keyboardKey);

                            std::stringstream l_transformStream(l_attribTransform.as_string("0 0 0 0"));
                            glm::uvec4 l_transform(0U);
                            l_transformStream >> l_transform[0] >> l_transform[1] >> l_transform[2] >> l_transform[3];
                            l_guiButton->SetPosition(sf::Vector2f(static_cast<float>(l_transform[0]),static_cast<float>(l_transform[1])));
                            l_guiButton->SetSize(sf::Vector2f(static_cast<float>(l_transform[2]), static_cast<float>(l_transform[3])));

                            l_guiButton->SetText(l_keyboardKey->m_text);
                            l_guiButton->SetTextSize(24U);
                            l_guiButton->SetSelectionColor(g_hoverColor);
                            l_guiButton->SetVisibility(true);
                            l_guiButton->SetClickCallback(l_clickCallback);

                            m_guiButtons.push_back(l_guiButton);
                        }
                    }
                }
            }
            delete l_layout;

#ifdef _WIN32
            int l_layoutsCount = GetKeyboardLayoutList(0,nullptr);
            glm::clamp(l_layoutsCount, 0, 2);
            if(l_layoutsCount > 0)
            {
                m_systemLayouts.assign(static_cast<size_t>(l_layoutsCount), nullptr);
                GetKeyboardLayoutList(l_layoutsCount, m_systemLayouts.data());
            }
#endif

            m_overlayMain->SetTexture(m_guiSystem->GetRenderTextureHandle());
        }

        m_valid = (m_overlayMain->IsValid() && m_guiSystem->IsValid());
        if(m_valid) m_overlayMain->Show();

        m_visible = m_valid;
    }

    return m_valid;
}

void WidgetKeyboard::Destroy()
{
    if(m_guiSystem)
    {
        for(auto l_button : m_guiButtons)
        {
            delete reinterpret_cast<KeyboadKey*>(l_button->GetUserPointer());
            m_guiSystem->Remove(l_button);
        }
        m_guiButtons.clear();

        delete m_guiSystem;
        m_guiSystem = nullptr;
    }

    m_activeMove = false;
    m_activePin = false;
    m_lastTriggerTick = 0U;

    Widget::Destroy();
}

void WidgetKeyboard::Update()
{
    if(m_valid && m_visible)
    {
        while(m_overlayMain->Poll(m_event))
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
                    m_guiSystem->ProcessClick(l_button, l_state, static_cast<unsigned int>(m_event.data.mouse.x), static_cast<unsigned int>(m_event.data.mouse.y));
                } break;
                case vr::VREvent_MouseMove:
                    m_guiSystem->ProcessMove(static_cast<unsigned int>(m_event.data.mouse.x), static_cast<unsigned int>(m_event.data.mouse.y));
                    break;
            }
        }

        if(m_activePin)
        {
            glm::quat l_handRot;
            VRDevicesStates::GetDeviceRotation(VRDeviceIndex::VDI_LeftController, l_handRot);
            const glm::quat l_rot = glm::rotate(l_handRot, -g_piHalf, g_axisX);
            m_overlayMain->GetTransform()->SetRotation(l_rot);

            glm::vec3 l_handPos;
            VRDevicesStates::GetDevicePosition(VRDeviceIndex::VDI_LeftController, l_handPos);
            m_overlayMain->GetTransform()->SetPosition(l_handPos);
        }

        m_guiSystem->Update();
        m_overlayMain->Update();

        if(m_activeDashboard)
        {
            // TODO
        }
    }
}

void WidgetKeyboard::OnHandDeactivated(size_t f_hand)
{
    Widget::OnHandDeactivated(f_hand);

    if(m_valid)
    {
        if((f_hand == VRDeviceIndex::VDI_LeftController) && m_activeMove) m_activeMove = false;
    }
}

void WidgetKeyboard::OnButtonPress(size_t f_hand, uint32_t f_button)
{
    Widget::OnButtonPress(f_hand, f_button);
}

void WidgetKeyboard::OnDashboardOpen()
{
    Widget::OnDashboardOpen();

    if(m_valid)
    {
        if(!m_activePin) m_overlayMain->Show();
    }
}
void WidgetKeyboard::OnDashboardClose()
{
    Widget::OnDashboardClose();

    if(m_valid)
    {
        if(!m_activePin) m_overlayMain->Hide();
    }
}

void WidgetKeyboard::OnGuiElementClick_Keys(GuiElement *f_guiElement, unsigned char f_button, unsigned char f_state)
{
    if((f_button == GuiClick::GC_Left) && (f_state == GuiClickState::GCS_Press))
    {
        KeyboadKey *l_keyboardKey = reinterpret_cast<KeyboadKey*>(f_guiElement->GetUserPointer());
        if(l_keyboardKey)
        {
#ifdef _WIN32
            INPUT l_input;
            l_input.type = INPUT_KEYBOARD;
            l_input.ki.wScan = 0;
            l_input.ki.time = 0;
            l_input.ki.dwExtraInfo = 0;
            l_input.ki.wVk = l_keyboardKey->m_code;
            l_input.ki.dwFlags = 0;
            SendInput(1U, &l_input, sizeof(INPUT));

            l_input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1U, &l_input, sizeof(INPUT));
#elif __linux__
            // Implement in Linux way
#endif
        }
    }
}

void WidgetKeyboard::InitStaticResources()
{
    if(!ms_textureAtlas)
    {
        ms_textureAtlas = new sf::Texture();
        if(!ms_textureAtlas->loadFromFile("icons/atlas_keyboard.png")) ms_textureAtlas->loadFromMemory(g_dummyTextureData, 16U);
    }
}

void WidgetKeyboard::RemoveStaticResources()
{
    if(ms_textureAtlas)
    {
        delete ms_textureAtlas;
        ms_textureAtlas = nullptr;
    }
}
