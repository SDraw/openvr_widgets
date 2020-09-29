#include "stdafx.h"

#include "Widgets/WidgetKeyboard.h"
#include "Gui/GuiSystem.h"
#include "Gui/GuiButton.h"
#include "Utils/Transformation.h"

#include "Core/VRDevicesStates.h"
#include "Managers/ConfigManager.h"
#include "Gui/GuiStructures.h"
#include "Utils/Utils.h"

extern const float g_piHalf;
extern const glm::vec3 g_axisX;
extern const glm::vec3 g_axisZN;
extern const unsigned char g_dummyTextureData[];

const sf::Vector2u g_targetSize(1024U, 512U);
const sf::Color g_hoverColor(142U, 205U, 246U);
const sf::Vector2f g_buttonSize(64.f, 64.f);

const unsigned short g_keyCodes[]
{
#ifdef _WIN32
    VK_ESCAPE, VK_PRINT, VK_PAUSE,
        VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,
        VK_OEM_3, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, VK_OEM_MINUS, VK_OEM_PLUS,
        0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4F, 0x50, VK_OEM_4, VK_OEM_6,
        0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B, 0x4C, VK_OEM_1, VK_OEM_7, VK_OEM_8,
        VK_OEM_102, 0x5A, 0x58, 0x43, 0x56, 0x42, 0x4E, 0x4D, VK_OEM_PERIOD, VK_OEM_COMMA, VK_OEM_2,
        VK_SPACE,
        VK_LSHIFT, VK_RSHIFT,
        VK_LCONTROL, VK_RCONTROL, VK_MENU, VK_MENU,
        VK_BACK, VK_INSERT,
        VK_RETURN, VK_DELETE,
        VK_RETURN,
        VK_TAB,
        VK_HOME, VK_PRIOR,
        VK_END, VK_NEXT,
        VK_UP,
        VK_LEFT, VK_DOWN, VK_RIGHT,
        VK_DIVIDE, VK_MULTIPLY, VK_SUBTRACT,
        VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9, VK_ADD,
        VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6,
        VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3,
        VK_NUMPAD0, VK_OEM_COMMA,
        VK_SCROLL,
        VK_NUMLOCK,
        VK_CAPITAL
#elif __linux__
    // Implement in Linux way
#endif
};
const char *g_KeyNames[]
{
    "Esc", "PrtSc", "Pause",
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
        "~", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=",
        "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]",
        "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "#", // Last one is unknown, 
        "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/",
        "Space",
        "LShift", "RShift",
        "LCtrl", "RCtrl", "Alt", "AltGr",
        "Backspace", "Insert",
        "Enter", "Delete",
        "NumEnter",
        "Tab",
        "Home", "PgUp",
        "End", "PgDown",
        "Up",
        "Left", "Down", "Right",
        "/", "*", "-",
        "Num7", "Num8", "Num9", "+",
        "Num4", "Num5", "Num6",
        "Num1", "Num2", "Num3",
        "Num0", "Delete",
        "ScrLk",
        "NumLk",
        "CapsLk"
};

sf::Texture *WidgetKeyboard::ms_textureAtlas = nullptr;
vr::Texture_t WidgetKeyboard::ms_textureControls = { 0 };

WidgetKeyboard::WidgetKeyboard()
{
    for(size_t i = 0U; i < CI_Count; i++)
    {
        m_overlayControls[i] = vr::k_ulOverlayHandleInvalid;
        m_transformControls[i] = nullptr;
    }
    m_guiSystem = nullptr;
    for(size_t i = 0U; i < KKI_Count; i++) m_guiButtons[i] = nullptr;
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
        m_guiSystem = new GuiSystem(g_targetSize);
        if(m_guiSystem->IsValid())
        {
            m_guiSystem->SetFont(ConfigManager::GetGuiFont());

            for(size_t i = 0U; i < CI_Count; i++) m_transformControls[i] = new Transformation();

            const auto l_clickCallback = std::bind(&WidgetKeyboard::OnGuiElementClick, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

            // 7 lines, 16 keys per line
            for(size_t i = 0U; i < KKI_Count; i++)
            {
                m_guiButtons[i] = m_guiSystem->CreateButton();

                const size_t l_field = (i % 16U);
                const size_t l_line = (i - l_field) / 16U;
                const sf::Vector2f l_buttonPosition(l_field*64.f, l_line*64.f);
                m_guiButtons[i]->SetPosition(l_buttonPosition);
                m_guiButtons[i]->SetSize(g_buttonSize);
                m_guiButtons[i]->SetSelectionColor(g_hoverColor);
                m_guiButtons[i]->SetText(g_KeyNames[i]);
                m_guiButtons[i]->SetTextSize(24U);
                m_guiButtons[i]->SetVisibility(true);
                m_guiButtons[i]->SetClickCallback(l_clickCallback);
                m_guiButtons[i]->SetUserPointer(reinterpret_cast<void*>(i));
            }
        }

        std::string l_overlayKeyPart("ovrw.keyboard_");
        l_overlayKeyPart.append(std::to_string(reinterpret_cast<size_t>(this)));

        std::string l_overlayKeyFull(l_overlayKeyPart);
        l_overlayKeyFull.append(".main");
        vr::VROverlay()->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Keyboard - Main", &m_overlay);
        if(m_overlay != vr::k_ulOverlayHandleInvalid)
        {
            // Create overlay in front of user
            glm::vec3 l_hmdPos;
            glm::quat l_hmdRot;
            VRDevicesStates::GetDevicePosition(VRDeviceIndex::VDI_Hmd, l_hmdPos);
            VRDevicesStates::GetDeviceRotation(VRDeviceIndex::VDI_Hmd, l_hmdRot);
            glm::vec3 l_pos = l_hmdPos + (l_hmdRot*g_axisZN)*0.5f;
            m_transform->SetPosition(l_pos);

            glm::quat l_rot;
            GetRotationToPoint(l_hmdPos, l_pos, l_hmdRot, l_rot);
            m_transform->SetRotation(l_rot);

            vr::VROverlay()->SetOverlayInputMethod(m_overlay, vr::VROverlayInputMethod_Mouse);
            vr::VROverlay()->SetOverlayWidthInMeters(m_overlay, 1.0f);

            const vr::HmdVector2_t l_mouseScale = { 1024.f, 512.f };
            vr::VROverlay()->SetOverlayMouseScale(m_overlay, &l_mouseScale);
            vr::VROverlay()->SetOverlayWidthInMeters(m_overlay, 0.5f);

            m_texture.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(m_guiSystem->GetRenderTextureHandle()));
            vr::VROverlay()->ShowOverlay(m_overlay);
        }

        l_overlayKeyFull.assign(l_overlayKeyPart);
        l_overlayKeyFull.append(".pin");
        vr::VROverlay()->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Keyboard - Pin", &m_overlayControls[CI_Pin]);
        if(m_overlayControls[CI_Pin] != vr::k_ulOverlayHandleInvalid)
        {
            m_transformControls[CI_Pin]->SetPosition(glm::vec3(0.285f, 0.03f, 0.f));
            vr::VRTextureBounds_t l_bounds = { 0.f, 1.f, 0.5f, 0.5f };
            vr::VROverlay()->SetOverlayTextureBounds(m_overlayControls[CI_Pin], &l_bounds);
        }

        l_overlayKeyFull.assign(l_overlayKeyPart);
        l_overlayKeyFull.append(".close");
        vr::VROverlay()->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Keyboard - Close", &m_overlayControls[CI_Close]);
        if(m_overlayControls[CI_Close] != vr::k_ulOverlayHandleInvalid)
        {
            m_transformControls[CI_Close]->SetPosition(glm::vec3(0.285f, -0.03f, 0.f));
            vr::VRTextureBounds_t l_bounds = { 0.f, 0.5f, 0.5f, 0.f };
            vr::VROverlay()->SetOverlayTextureBounds(m_overlayControls[CI_Close], &l_bounds);
        }

        m_valid = (m_overlay != vr::k_ulOverlayHandleInvalid && m_guiSystem->IsValid());
        for(size_t i = 0U; i < CI_Count; i++)
        {
            if(m_overlayControls[i] != vr::k_ulOverlayHandleInvalid)
            {
                vr::VROverlay()->SetOverlayWidthInMeters(m_overlayControls[i], 0.05f);
                vr::VROverlay()->SetOverlayInputMethod(m_overlayControls[i], vr::VROverlayInputMethod::VROverlayInputMethod_Mouse);
            }
            else
            {
                m_valid = false;
                break;
            }
        }

        m_visible = m_valid;
    }

    return m_valid;
}

void WidgetKeyboard::Destroy()
{
    if(m_guiSystem)
    {
        for(size_t i = 0U; i < KKI_Count; i++)
        {
            if(m_guiButtons[i])
            {
                m_guiSystem->Remove(m_guiButtons[i]);
                m_guiButtons[i] = nullptr;
            }
        }

        delete m_guiSystem;
        m_guiSystem = nullptr;
    }

    for(size_t i = 0U; i < CI_Count; i++)
    {
        if(m_overlayControls[i] != vr::k_ulOverlayHandleInvalid)
        {
            vr::VROverlay()->ClearOverlayTexture(m_overlayControls[i]);
            vr::VROverlay()->DestroyOverlay(m_overlayControls[i]);
            m_overlayControls[i] = vr::k_ulOverlayHandleInvalid;
        }
        delete m_transformControls[i];
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
        while(vr::VROverlay()->PollNextOverlayEvent(m_overlay, &m_event, sizeof(vr::VREvent_t)))
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

        while(vr::VROverlay()->PollNextOverlayEvent(m_overlayControls[CI_Pin], &m_event, sizeof(vr::VREvent_t)))
        {
            switch(m_event.eventType)
            {
                case vr::VREvent_MouseButtonDown:
                {
                    m_activePin = !m_activePin;
                    //ms_vrOverlay->SetOverlayFlag(m_overlay, vr::VROverlayFlags_MakeOverlaysInteractiveIfVisible, m_activePin);

                    vr::VRTextureBounds_t l_bounds;
                    if(m_activePin) l_bounds = { 0.5f, 1.0f, 1.0f, 0.5f };
                    else l_bounds = { 0.f, 1.f, 0.5f, 0.5f };
                    vr::VROverlay()->SetOverlayTextureBounds(m_overlayControls[CI_Pin], &l_bounds);
                } break;
            }
        }

        while(vr::VROverlay()->PollNextOverlayEvent(m_overlayControls[CI_Close], &m_event, sizeof(vr::VREvent_t)))
        {
            switch(m_event.eventType)
            {
                case vr::VREvent_MouseButtonDown:
                    m_closed = true;
                    break;
            }
        }

        if(m_activePin)
        {
            glm::quat l_handRot;
            VRDevicesStates::GetDeviceRotation(VRDeviceIndex::VDI_LeftController, l_handRot);
            const glm::quat l_rot = glm::rotate(l_handRot, -g_piHalf, g_axisX);
            m_transform->SetRotation(l_rot);

            glm::vec3 l_handPos;
            VRDevicesStates::GetDevicePosition(VRDeviceIndex::VDI_LeftController, l_handPos);
            m_transform->SetPosition(l_handPos);
        }
        m_transform->Update();
        vr::VROverlay()->SetOverlayTransformAbsolute(m_overlay, vr::TrackingUniverseRawAndUncalibrated, &m_transform->GetMatrixVR());

        m_guiSystem->Update();
        vr::VROverlay()->SetOverlayTexture(m_overlay, &m_texture);

        if(m_activeDashboard)
        {
            for(size_t i = 0U; i < CI_Count; i++)
            {
                m_transformControls[i]->Update(m_transform);
                vr::VROverlay()->SetOverlayTransformAbsolute(m_overlayControls[i], vr::TrackingUniverseRawAndUncalibrated, &m_transformControls[i]->GetMatrixVR());
                vr::VROverlay()->SetOverlayTexture(m_overlayControls[i], &ms_textureControls);
            }
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

    if(m_valid && !m_activeDashboard && m_visible)
    {
        if((f_hand == VRDeviceIndex::VDI_LeftController) && (f_button == vr::k_EButton_SteamVR_Trigger))
        {
            const unsigned long long l_tick = GetTickCount64();
            if(l_tick - m_lastTriggerTick < 500U)
            {
                if(!m_activeMove)
                {
                    glm::vec3 l_handPos;
                    VRDevicesStates::GetDevicePosition(VRDeviceIndex::VDI_LeftController, l_handPos);
                    m_activeMove = (glm::distance(m_transform->GetPosition(), l_handPos) <= 0.1f);
                }
                else m_activeMove = false;
            }
            m_lastTriggerTick = l_tick;
        }
    }
}

void WidgetKeyboard::OnDashboardOpen()
{
    Widget::OnDashboardOpen();

    if(m_valid)
    {
        for(size_t i = 0U; i < CI_Count; i++) vr::VROverlay()->ShowOverlay(m_overlayControls[i]);
        if(!m_activePin) vr::VROverlay()->ShowOverlay(m_overlay);
    }
}
void WidgetKeyboard::OnDashboardClose()
{
    Widget::OnDashboardClose();

    if(m_valid)
    {
        for(size_t i = 0U; i < CI_Count; i++) vr::VROverlay()->HideOverlay(m_overlayControls[i]);
        if(!m_activePin) vr::VROverlay()->HideOverlay(m_overlay);
    }
}

void WidgetKeyboard::OnGuiElementClick(GuiElement *f_guiElement, unsigned char f_button, unsigned char f_state)
{
    if((f_button == GuiClick::GC_Left) && (f_state == GuiClickState::GCS_Press))
    {
#ifdef _WIN32
        INPUT l_input;
        l_input.type = INPUT_KEYBOARD;
        l_input.ki.wScan = 0;
        l_input.ki.time = 0;
        l_input.ki.dwExtraInfo = 0;
        l_input.ki.wVk = g_keyCodes[reinterpret_cast<size_t>(f_guiElement->GetUserPointer())];
        l_input.ki.dwFlags = 0;
        SendInput(1U, &l_input, sizeof(INPUT));

        l_input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1U, &l_input, sizeof(INPUT));
#elif __linux__
        // Implement in Linux way
#endif
    }
}

void WidgetKeyboard::InitStaticResources()
{
    if(!ms_textureAtlas)
    {
        ms_textureAtlas = new sf::Texture();
        if(!ms_textureAtlas->loadFromFile("icons/atlas_keyboard.png")) ms_textureAtlas->loadFromMemory(g_dummyTextureData, 16U);

        ms_textureControls.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(ms_textureAtlas->getNativeHandle()));
        ms_textureControls.eType = vr::TextureType_OpenGL;
        ms_textureControls.eColorSpace = vr::ColorSpace_Gamma;
    }
}

void WidgetKeyboard::RemoveStaticResources()
{
    if(ms_textureAtlas)
    {
        delete ms_textureAtlas;
        ms_textureAtlas = nullptr;
        ms_textureControls = { 0 };
    }
}
