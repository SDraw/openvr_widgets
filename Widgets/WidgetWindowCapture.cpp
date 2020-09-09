#include "stdafx.h"

#include "Widgets/WidgetWindowCapture.h"
#include "Gui/GuiSystem.h"
#include "Gui/GuiImage.h"
#include "Gui/GuiText.h"
#include "Utils/Transformation.h"
#include "Utils/WindowCapturer.h"

#include "Core/GlobalSettings.h"
#include "Core/VRDevicesStates.h"
#include "Gui/GuiStructures.h"
#include "Utils/Utils.h"

extern const float g_PiHalf;
extern const glm::ivec2 g_EmptyIVector2;
extern const glm::vec3 g_AxisX;
extern const glm::vec3 g_AxisZN;
extern const sf::Color g_ClearColor;
extern const unsigned char g_DummyTextureData[];

const size_t g_CaptureDelays[3U]
{
    66U, 33U, 16U
};
const sf::Vector2u g_GuiSystemDefaultSize(256U, 448U);
const sf::Vector2i g_GuiButtonsDefaultSize(128, 128);
const sf::Vector2f g_GuiButtonsInitialPositions[6U]
{
    { 0.f, 64.f },
    { 128.f, 64.f },
    { 0.f, 192.f },
    { 128.f, 192.f },
    { 0.f, 320.f },
    { 128.f, 320.f }
};
const sf::Vector2i g_GuiButtonsInitialUV[6U]
{
    { 0, 0 },
    { 256, 0 },
    { 384, 0 },
    { 0, 128 },
    { 128, 128 },
    { 256, 128 },
};
const sf::Vector2i g_GuiButtonPinUV[2U]
{
    { 0, 0 },
    { 128, 0 }
};
const sf::Vector2i g_GuiButtonFpsUV[3U]
{
    {256, 128},
    { 384, 128 },
    { 0, 256 }
};

sf::Texture *WidgetWindowCapture::ms_textureAtlas = nullptr;
#ifdef __linux__
Display *WidgetWindowCapture::ms_display = nullptr;
#endif

WidgetWindowCapture::WidgetWindowCapture()
{
    m_overlayControl = vr::k_ulOverlayHandleInvalid;

    m_windowCapturer = nullptr;
    m_windowIndex = std::numeric_limits<size_t>::max();

    m_guiSystem = nullptr;
    for(size_t i = 0U; i < CEI_Count; i++) m_guiImages[i] = nullptr;
    m_guiTextWindow = nullptr;

    m_textureControls = { 0 };

    m_lastLeftTriggerTick = 0U;
    m_lastRightTriggerTick = 0U;

    m_activeMove = false;
    m_activeResize = false;
    m_activePin = false;

    m_overlayWidth = 0.f;
    m_windowSize = g_EmptyIVector2;
    m_mousePosition = g_EmptyIVector2;
    m_transformControl = nullptr;

    m_fpsMode = FM_15;
}
WidgetWindowCapture::~WidgetWindowCapture()
{
}

bool WidgetWindowCapture::Create()
{
    if(!m_valid)
    {
        m_overlayWidth = 0.5f;

#ifdef __linux__
        if(!ms_display) ms_display = XOpenDisplay(nullptr);
#endif

        if(!ms_textureAtlas)
        {
            ms_textureAtlas = new sf::Texture();
            if(!ms_textureAtlas->loadFromFile("icons/atlas_capture.png")) ms_textureAtlas->loadFromMemory(g_DummyTextureData, 16U);
        }

        std::string l_overlayKeyPart("ovrw.capture_");
        l_overlayKeyPart.append(std::to_string(reinterpret_cast<size_t>(this)));
        std::string l_overlayKeyFull(l_overlayKeyPart);

        l_overlayKeyFull.append(".main");
        vr::VROverlay()->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Capture - Main", &m_overlay);
        if(m_overlay != vr::k_ulOverlayHandleInvalid)
        {
            // Create overlay in front of user
            glm::vec3 l_hmdPos;
            glm::quat l_hmdRot;
            VRDevicesStates::GetDevicePosition(VRDeviceIndex::VDI_Hmd, l_hmdPos);
            VRDevicesStates::GetDeviceRotation(VRDeviceIndex::VDI_Hmd, l_hmdRot);

            glm::vec3 l_pos = l_hmdPos + (l_hmdRot*g_AxisZN)*0.5f;
            m_transform->SetPosition(l_pos);

            glm::quat l_rot;
            GetRotationToPoint(l_hmdPos, l_pos, l_hmdRot, l_rot);
            m_transform->SetRotation(l_rot);
            m_texture.handle = nullptr;

            vr::VROverlay()->SetOverlayWidthInMeters(m_overlay, m_overlayWidth);
            vr::VROverlay()->SetOverlayFlag(m_overlay, vr::VROverlayFlags_SortWithNonSceneOverlays, true);
            vr::VROverlay()->SetOverlayFlag(m_overlay, vr::VROverlayFlags_ProtectedContent, true);
            vr::VROverlay()->SetOverlayInputMethod(m_overlay, vr::VROverlayInputMethod_Mouse);
            vr::VROverlay()->SetOverlayFlag(m_overlay, vr::VROverlayFlags_ShowTouchPadScrollWheel, true);
            vr::VROverlay()->SetOverlayFlag(m_overlay, vr::VROverlayFlags_SendVRDiscreteScrollEvents, true);
            vr::VROverlay()->ShowOverlay(m_overlay);
        }

        l_overlayKeyFull.assign(l_overlayKeyPart);
        l_overlayKeyFull.append(".control");
        vr::VROverlay()->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Capture - Control", &m_overlayControl);
        if(m_overlayControl != vr::k_ulOverlayHandleInvalid)
        {
            m_guiSystem = new GuiSystem(g_GuiSystemDefaultSize);
            m_guiSystem->SetFont(GlobalSettings::GetGuiFont());

            const std::function<void(GuiElement*, unsigned char, unsigned char, unsigned int, unsigned int)> l_clickCallback([this](GuiElement *f_guiElement, unsigned char f_button, unsigned char f_state, unsigned int, unsigned int)
            {
                this->OnGuiElementMouseClick(f_guiElement, f_button, f_state);
            });
            for(size_t i = 0U; i < CEI_Count; i++)
            {
                m_guiImages[i] = m_guiSystem->CreateImage(ms_textureAtlas);
                m_guiImages[i]->SetPosition(g_GuiButtonsInitialPositions[i]);
                m_guiImages[i]->SetSize(sf::Vector2f(g_GuiButtonsDefaultSize));
                m_guiImages[i]->SetUV(g_GuiButtonsInitialUV[i], g_GuiButtonsDefaultSize);
                m_guiImages[i]->SetUserPointer(reinterpret_cast<void*>(i));
                m_guiImages[i]->SetClickCallback(l_clickCallback);
            }
            m_guiTextWindow = m_guiSystem->CreateText();
            m_guiTextWindow->Set("<>");
            m_guiTextWindow->SetCharactersSize(16U);
            m_guiTextWindow->SetAlignment(GuiText::GTA_Center);
            m_guiTextWindow->SetPosition(sf::Vector2f(128.f, 32.f));

            m_textureControls.eType = vr::TextureType_OpenGL;
            m_textureControls.eColorSpace = vr::ColorSpace_Gamma;
            m_textureControls.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(m_guiSystem->GetRenderTextureHandle()));

            m_transformControl = new Transformation();
            m_transformControl->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.072f, 0.f, 0.f));

            vr::HmdVector2_t l_mouseScale;
            l_mouseScale.v[0U] = static_cast<float>(g_GuiSystemDefaultSize.x);
            l_mouseScale.v[1U] = static_cast<float>(g_GuiSystemDefaultSize.y);
            vr::VROverlay()->SetOverlayMouseScale(m_overlayControl, &l_mouseScale);
            vr::VROverlay()->SetOverlayFlag(m_overlayControl, vr::VROverlayFlags_SortWithNonSceneOverlays, true);
            vr::VROverlay()->SetOverlayInputMethod(m_overlayControl, vr::VROverlayInputMethod_Mouse);
            vr::VROverlay()->SetOverlayWidthInMeters(m_overlayControl, 0.128f);

            vr::VROverlay()->ShowOverlay(m_overlayControl);
        }

        m_valid = ((m_overlay != vr::k_ulOverlayHandleInvalid) && (m_overlayControl != vr::k_ulOverlayHandleInvalid));
        if(m_valid)
        {
            // Create window grabber and start capture
            m_windowCapturer = new WindowCapturer();
            m_windowCapturer->UpdateWindows();
            m_windowIndex = 0U;
            StartCapture();

            m_visible = true;
        }
    }

    return m_valid;
}

void WidgetWindowCapture::Destroy()
{
    if(m_overlayControl != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->HideOverlay(m_overlayControl);
        vr::VROverlay()->ClearOverlayTexture(m_overlayControl);
        vr::VROverlay()->DestroyOverlay(m_overlayControl);
        m_overlayControl = vr::k_ulOverlayHandleInvalid;
    }

    delete m_guiSystem;
    m_guiSystem = nullptr;
    for(size_t i = 0U; i < CEI_Count; i++) m_guiImages[i] = nullptr;
    m_guiTextWindow = nullptr;

    m_textureControls = { 0 };

    delete m_transformControl;
    m_transformControl = nullptr;

    delete m_windowCapturer;
    m_windowCapturer = nullptr;

    Widget::Destroy();
}

void WidgetWindowCapture::Update()
{
    if(m_valid && m_visible)
    {
        // Poll overlays interaction
        while(vr::VROverlay()->PollNextOverlayEvent(m_overlay, &m_event, sizeof(vr::VREvent_t)))
        {
            switch(m_event.eventType)
            {
                case vr::VREvent_MouseMove:
                {
                    m_mousePosition.x = static_cast<int>(m_event.data.mouse.x);
                    m_mousePosition.y = static_cast<int>(m_event.data.mouse.y);
                } break;

                case vr::VREvent_MouseButtonDown:
                {
                    const auto *l_window = m_windowCapturer->GetWindowInfo(m_windowIndex);
                    if(l_window)
                    {
#ifdef _WIN32
                        if(IsWindow(reinterpret_cast<HWND>(l_window->Handle)))
                        {
                            DWORD l_buttonData[3] = { 0 };
                            switch(m_event.data.mouse.button)
                            {
                                case vr::VRMouseButton_Left:
                                {
                                    l_buttonData[0] = MK_LBUTTON;
                                    l_buttonData[1] = WM_LBUTTONDOWN;
                                    l_buttonData[2] = WM_LBUTTONUP;
                                } break;
                                case vr::VRMouseButton_Right:
                                {
                                    l_buttonData[0] = MK_RBUTTON;
                                    l_buttonData[1] = WM_RBUTTONDOWN;
                                    l_buttonData[2] = WM_RBUTTONUP;
                                } break;
                                case vr::VRMouseButton_Middle:
                                {
                                    l_buttonData[0] = MK_MBUTTON;
                                    l_buttonData[1] = WM_MBUTTONDOWN;
                                    l_buttonData[2] = WM_MBUTTONUP;
                                } break;
                            }

                            SendMessage(reinterpret_cast<HWND>(l_window->Handle), WM_MOUSEMOVE, NULL, MAKELPARAM(m_mousePosition.x, m_mousePosition.y));
                            SendMessage(reinterpret_cast<HWND>(l_window->Handle), l_buttonData[1], l_buttonData[0], MAKELPARAM(m_mousePosition.x, m_mousePosition.y));
                            SendMessage(reinterpret_cast<HWND>(l_window->Handle), l_buttonData[2], NULL, MAKELPARAM(m_mousePosition.x, m_mousePosition.y));
                        }
#elif __linux__
                        if(ms_display)
                        {
                            XEvent l_event = { 0 };

                            switch(m_event.data.mouse.button)
                            {
                                case vr::VRMouseButton_Left:
                                    l_event.xbutton.button = Button1;
                                    break;
                                case vr::VRMouseButton_Right:
                                    l_event.xbutton.button = Button3;
                                    break;
                                case vr::VRMouseButton_Middle:
                                    l_event.xbutton.button = Button2;
                                    break;
                            }
                            l_event.xbutton.same_screen = true;
                            l_event.xbutton.window = l_window->Handle;
                            l_event.xbutton.x = m_mousePosition.x;
                            l_event.xbutton.y = m_mousePosition.y;

                            l_event.type = ButtonPress;
                            XSendEvent(ms_display, l_window->Handle, true, ButtonPressMask, &l_event);
                            XFlush(ms_display);

                            l_event.type = ButtonRelease;
                            XSendEvent(ms_display, l_window->Handle, true, ButtonReleaseMask, &l_event);
                            XFlush(ms_display);
                        }
#endif
                    }
                } break;

                case vr::VREvent_ScrollDiscrete:
                {
                    const auto *l_window = m_windowCapturer->GetWindowInfo(m_windowIndex);
                    if(l_window)
                    {
#ifdef _WIN32
                        if(IsWindow(reinterpret_cast<HWND>(l_window->Handle)))
                        {
                            SendMessage(reinterpret_cast<HWND>(l_window->Handle), WM_MOUSEMOVE, NULL, MAKELPARAM(m_mousePosition.x, m_mousePosition.y));
                            SendMessage(reinterpret_cast<HWND>(l_window->Handle), WM_MOUSEWHEEL, MAKEWPARAM(NULL, m_event.data.scroll.ydelta * WHEEL_DELTA), MAKELPARAM(m_mousePosition.x, m_mousePosition.y));
                        }
#elif __linux__
                        if(ms_display)
                        {
                            XEvent l_event = { 0 };

                            l_event.xbutton.button = (m_event.data.scroll.ydelta < 0.f) ? Button5 : Button4;
                            l_event.xbutton.same_screen = true;
                            l_event.xbutton.window = l_window->Handle;
                            l_event.xbutton.x = m_mousePosition.x;
                            l_event.xbutton.y = m_mousePosition.y;

                            l_event.type = ButtonPress;
                            XSendEvent(ms_display, l_window->Handle, true, ButtonPressMask, &l_event);
                            XFlush(ms_display);

                            l_event.type = ButtonRelease;
                            XSendEvent(ms_display, l_window->Handle, true, ButtonReleaseMask, &l_event);
                            XFlush(ms_display);
                    }
#endif
                }
            } break;
        }
    }

        while(vr::VROverlay()->PollNextOverlayEvent(m_overlayControl, &m_event, sizeof(vr::VREvent_t)))
        {
            switch(m_event.eventType)
            {
                case vr::VREvent_MouseMove:
#ifdef _WIN32
                    m_guiSystem->ProcessMove(static_cast<unsigned int>(m_event.data.mouse.x), static_cast<unsigned int>(m_event.data.mouse.y));
#elif __linux__
                    m_guiSystem->ProcessMove(static_cast<unsigned int>(m_event.data.mouse.x), static_cast<unsigned int>(g_GuiSystemDefaultSize.y - m_event.data.mouse.y));
#endif
                    break;

                case vr::VREvent_MouseButtonDown:
                {
                    if(m_event.data.mouse.button == vr::VRMouseButton_Left)
                    {
#ifdef _WIN32
                        m_guiSystem->ProcessClick(GuiClick::GC_Left, GuiClickState::GCS_Press, static_cast<unsigned int>(m_event.data.mouse.x), static_cast<unsigned int>(m_event.data.mouse.y));
#elif __linux__
                        m_guiSystem->ProcessClick(GuiClick::GC_Left, GuiClickState::GCS_Press, static_cast<unsigned int>(m_event.data.mouse.x), static_cast<unsigned int>(g_GuiSystemDefaultSize.y - m_event.data.mouse.y));
#endif
                    }
                } break;
            }
        }

        if(m_activeMove)
        {
            glm::quat l_handRot;
            VRDevicesStates::GetDeviceRotation(VRDeviceIndex::VDI_LeftController, l_handRot);
            const glm::quat l_rot = glm::rotate(l_handRot, -g_PiHalf, g_AxisX);
            m_transform->SetRotation(l_rot);

            glm::vec3 l_handPos;
            VRDevicesStates::GetDevicePosition(VRDeviceIndex::VDI_LeftController, l_handPos);
            m_transform->SetPosition(l_handPos);
        }
        if(m_activeResize)
        {
            glm::vec3 l_handPos;
            VRDevicesStates::GetDevicePosition(VRDeviceIndex::VDI_RightController, l_handPos);
            m_overlayWidth = (glm::distance(l_handPos, m_transform->GetPosition()) * 2.f);
            m_transformControl->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.072f, 0.f, 0.f));
            vr::VROverlay()->SetOverlayWidthInMeters(m_overlay, m_overlayWidth);
        }
        m_transform->Update();
        vr::VROverlay()->SetOverlayTransformAbsolute(m_overlay, vr::TrackingUniverseRawAndUncalibrated, &m_transform->GetMatrixVR());

        m_transformControl->Update(m_transform);
        if(m_activeDashboard)
        {
            m_guiSystem->Update();

            vr::VROverlay()->SetOverlayTransformAbsolute(m_overlayControl, vr::TrackingUniverseRawAndUncalibrated, &m_transformControl->GetMatrixVR());
            vr::VROverlay()->SetOverlayTexture(m_overlayControl, &m_textureControls);
        }

        if(m_windowCapturer->IsStale())
        {
            // Window was resized or destroyed
            m_windowCapturer->StopCapture();
            m_windowCapturer->UpdateWindows();
            m_windowIndex = 0U;
            StartCapture();
        }
        m_windowCapturer->Update();
        if(m_texture.handle) vr::VROverlay()->SetOverlayTexture(m_overlay, &m_texture);
}
}

void WidgetWindowCapture::OnButtonPress(size_t f_hand, uint32_t f_button)
{
    Widget::OnButtonPress(f_hand, f_button);

    if(m_valid)
    {
        switch(f_hand)
        {
            case VRDeviceIndex::VDI_LeftController:
            {
                switch(f_button)
                {
                    case vr::k_EButton_SteamVR_Trigger:
                    {
                        if(m_visible && !m_activeDashboard && !m_activePin)
                        {
                            const unsigned long long l_tick = GetTickCount64();
                            if((l_tick - m_lastLeftTriggerTick) < 500U)
                            {
                                if(!m_activeMove)
                                {
                                    glm::vec3 l_pos;
                                    VRDevicesStates::GetDevicePosition(VRDeviceIndex::VDI_LeftController, l_pos);
                                    m_activeMove = (glm::distance(l_pos, m_transform->GetPosition()) < (m_overlayWidth * 0.2f));
                                }
                                else m_activeMove = false;
                            }
                            m_lastLeftTriggerTick = l_tick;
                        }
                    } break;
                }
            } break;

            case VRDeviceIndex::VDI_RightController:
            {
                if(m_activeMove && (f_button == vr::k_EButton_SteamVR_Trigger))
                {
                    const unsigned long long l_tick = GetTickCount64();
                    if((l_tick - m_lastRightTriggerTick) < 500U)
                    {
                        glm::vec3 l_pos;
                        VRDevicesStates::GetDevicePosition(VRDeviceIndex::VDI_RightController, l_pos);
                        m_activeResize = (glm::distance(l_pos, m_transform->GetPosition()) <= (m_overlayWidth * 0.5f));
                    }
                    m_lastRightTriggerTick = l_tick;
                }
            } break;
        }
    }
}

void WidgetWindowCapture::OnButtonRelease(size_t f_hand, uint32_t f_button)
{
    Widget::OnButtonRelease(f_hand, f_button);

    if(m_valid && m_visible)
    {
        if((f_hand == VRDeviceIndex::VDI_RightController) && (f_button == vr::k_EButton_SteamVR_Trigger))
        {
            if(m_activeResize) m_activeResize = false;
        }
    }
}

void WidgetWindowCapture::OnDashboardOpen()
{
    Widget::OnDashboardOpen();

    if(m_valid && m_visible)
    {
        m_activeMove = false;
        m_activeResize = false;

        vr::VROverlay()->ShowOverlay(m_overlayControl);
    }
}
void WidgetWindowCapture::OnDashboardClose()
{
    Widget::OnDashboardClose();

    if(m_valid && m_visible) vr::VROverlay()->HideOverlay(m_overlayControl);
}

void WidgetWindowCapture::StartCapture()
{
    if(m_windowCapturer->StartCapture(m_windowIndex))
    {
        m_texture.handle = m_windowCapturer->GetTextureHandle();
        const auto *l_window = m_windowCapturer->GetWindowInfo(m_windowIndex);
        if(l_window)
        {
            const vr::HmdVector2_t l_scale = { static_cast<float>(l_window->Size.x), static_cast<float>(l_window->Size.y) };
            vr::VROverlay()->SetOverlayMouseScale(m_overlay, &l_scale);

#ifdef _WIN32
            wchar_t l_windowName[256U];
            if(GetWindowTextW(reinterpret_cast<HWND>(l_window->Handle), l_windowName, 256) != 0) m_guiTextWindow->Set(l_windowName);
            else m_guiTextWindow->Set("<>");
#elif __linux__
            m_guiTextWindow->Set(l_window->Name);
#endif
        }
    }
    else m_texture.handle = nullptr;
}

void WidgetWindowCapture::OnGuiElementMouseClick(GuiElement *f_guiElement, unsigned char f_button, unsigned char f_state)
{
    if((f_button == GuiClick::GC_Left) && (f_state == GuiClickState::GCS_Press))
    {
        switch(reinterpret_cast<size_t>(f_guiElement->GetUserPointer()))
        {
            case CEI_Pin:
            {
                m_activePin = !m_activePin;
                m_guiImages[CEI_Pin]->SetUV(g_GuiButtonPinUV[m_activePin ? 1U : 0U], g_GuiButtonsDefaultSize);
            } break;

            case CEI_Close:
                m_closed = true;
                break;

            case CEI_Previous:
            {
                const size_t l_windowsCount = m_windowCapturer->GetWindowsCount();
                if(l_windowsCount > 0U)
                {
                    m_windowIndex += (l_windowsCount - 1U);
                    m_windowIndex %= l_windowsCount;

                    m_windowCapturer->StopCapture();
                    StartCapture();
                }
            } break;

            case CEI_Next:
            {
                const size_t l_windowsCount = m_windowCapturer->GetWindowsCount();
                if(l_windowsCount > 0U)
                {
                    m_windowIndex += 1U;
                    m_windowIndex %= l_windowsCount;

                    m_windowCapturer->StopCapture();
                    StartCapture();
                }
            } break;

            case CEI_Update:
            {
                m_windowCapturer->StopCapture();
                m_windowCapturer->UpdateWindows();
                m_windowIndex = 0U;
                StartCapture();
            } break;
            case CEI_FPS:
            {
                m_fpsMode += 1U;
                m_fpsMode %= FM_Count;

                m_guiImages[CEI_FPS]->SetUV(g_GuiButtonFpsUV[m_fpsMode], g_GuiButtonsDefaultSize);
                m_windowCapturer->SetDelay(g_CaptureDelays[m_fpsMode]);
            } break;
        }
    }
}

// Static
void WidgetWindowCapture::RemoveStaticResources()
{
    if(ms_textureAtlas)
    {
        delete ms_textureAtlas;
        ms_textureAtlas = nullptr;
    }

#ifdef __linux__
    if(ms_display)
    {
        XCloseDisplay(ms_display);
        ms_display = nullptr;
    }
#endif

    WindowCapturer::RemoveStaticResources();
}
