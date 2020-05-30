#include "stdafx.h"

#include "Widgets/WidgetWindowCapture.h"
#include "Gui/GuiSystem.h"
#include "Gui/GuiImage.h"
#include "Gui/GuiText.h"
#include "Utils/Transformation.h"
#include "Utils/WindowGrabber.h"

#include "Core/GlobalSettings.h"
#include "Core/VRTransform.h"
#include "Gui/GuiStructures.h"
#include "Utils/GlobalStructures.h"
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

sf::Texture* WidgetWindowCapture::ms_textureAtlas = nullptr;

WidgetWindowCapture::WidgetWindowCapture()
{
    m_overlayControl = vr::k_ulOverlayHandleInvalid;

    m_windowGrabber = nullptr;
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

        if(!ms_textureAtlas)
        {
            ms_textureAtlas = new sf::Texture();
            if(!ms_textureAtlas->loadFromFile("icons/atlas_capture.png")) ms_textureAtlas->loadFromMemory(g_DummyTextureData, 16U);
        }

        std::string l_overlayKeyPart("ovrw.capture_");
        l_overlayKeyPart.append(std::to_string(reinterpret_cast<size_t>(this)));
        std::string l_overlayKeyFull(l_overlayKeyPart);

        l_overlayKeyFull.append(".main");
        ms_vrOverlay->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Capture - Main", &m_overlay);
        if(m_overlay != vr::k_ulOverlayHandleInvalid)
        {
            // Create overlay in front of camera
            glm::vec3 l_pos(VRTransform::GetHmdPosition());
            l_pos += (VRTransform::GetHmdRotation()*g_AxisZN)*0.5f;
            m_transform->SetPosition(l_pos);

            glm::quat l_rot;
            GetRotationToPoint(VRTransform::GetHmdPosition(), l_pos, VRTransform::GetHmdRotation(), l_rot);
            m_transform->SetRotation(l_rot);
            m_texture.handle = nullptr;

            ms_vrOverlay->SetOverlayWidthInMeters(m_overlay, m_overlayWidth);
            ms_vrOverlay->SetOverlayFlag(m_overlay, vr::VROverlayFlags_SortWithNonSceneOverlays, true);
            ms_vrOverlay->SetOverlayFlag(m_overlay, vr::VROverlayFlags_ProtectedContent, true);
            ms_vrOverlay->SetOverlayInputMethod(m_overlay, vr::VROverlayInputMethod_Mouse);
            ms_vrOverlay->SetOverlayFlag(m_overlay, vr::VROverlayFlags_ShowTouchPadScrollWheel, true);
            ms_vrOverlay->SetOverlayFlag(m_overlay, vr::VROverlayFlags_SendVRDiscreteScrollEvents, true);
            ms_vrOverlay->ShowOverlay(m_overlay);
        }

        l_overlayKeyFull.assign(l_overlayKeyPart);
        l_overlayKeyFull.append(".control");
        ms_vrOverlay->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Capture - Control", &m_overlayControl);
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
            m_guiTextWindow->Set("None");
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
            ms_vrOverlay->SetOverlayMouseScale(m_overlayControl, &l_mouseScale);
            ms_vrOverlay->SetOverlayFlag(m_overlayControl, vr::VROverlayFlags_SortWithNonSceneOverlays, true);
            ms_vrOverlay->SetOverlayInputMethod(m_overlayControl, vr::VROverlayInputMethod_Mouse);
            ms_vrOverlay->SetOverlayWidthInMeters(m_overlayControl, 0.128f);

            ms_vrOverlay->ShowOverlay(m_overlayControl);
        }

        m_valid = ((m_overlay != vr::k_ulOverlayHandleInvalid) && (m_overlayControl != vr::k_ulOverlayHandleInvalid));
        if(m_valid)
        {
            // Create window grabber and start capture
            m_windowGrabber = new WindowGrabber();
            m_windowGrabber->UpdateWindows();
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
        ms_vrOverlay->HideOverlay(m_overlayControl);
        ms_vrOverlay->ClearOverlayTexture(m_overlayControl);
        ms_vrOverlay->DestroyOverlay(m_overlayControl);
        m_overlayControl = vr::k_ulOverlayHandleInvalid;
    }

    delete m_guiSystem;
    m_guiSystem = nullptr;
    for(size_t i = 0U; i < CEI_Count; i++) m_guiImages[i] = nullptr;
    m_guiTextWindow = nullptr;

    m_textureControls = { 0 };

    delete m_transformControl;
    m_transformControl = nullptr;

    delete m_windowGrabber;
    m_windowGrabber = nullptr;

    Widget::Destroy();
}

void WidgetWindowCapture::Update()
{
    if(m_valid && m_visible)
    {
        // Poll overlays interaction
        while(ms_vrOverlay->PollNextOverlayEvent(m_overlay, &m_event, sizeof(vr::VREvent_t)))
        {
            switch(m_event.eventType)
            {
                case vr::VREvent_MouseMove:
                {
                    const auto *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
                    if(l_window)
                    {
                        if(IsWindow(reinterpret_cast<HWND>(l_window->Handle)))
                        {
                            m_mousePosition.x = static_cast<int>(m_event.data.mouse.x);
                            m_mousePosition.y = l_window->Size.y - static_cast<int>(m_event.data.mouse.y);
                        }
                    }
                } break;

                case vr::VREvent_MouseButtonDown:
                {
                    const auto *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
                    if(l_window)
                    {
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

                            const int l_posY = l_window->Size.y - m_mousePosition.y;
                            SendWinAPIMessage(reinterpret_cast<HWND>(l_window->Handle), WM_MOUSEMOVE, NULL, MAKELPARAM(m_mousePosition.x, l_posY));
                            SendWinAPIMessage(reinterpret_cast<HWND>(l_window->Handle), l_buttonData[1], l_buttonData[0], MAKELPARAM(m_mousePosition.x, l_posY));
                            SendWinAPIMessage(reinterpret_cast<HWND>(l_window->Handle), l_buttonData[2], NULL, MAKELPARAM(m_mousePosition.x, l_posY));
                        }
                    }
                } break;

                case vr::VREvent_ScrollDiscrete:
                {
                    const auto *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
                    if(l_window)
                    {
                        if(IsWindow(reinterpret_cast<HWND>(l_window->Handle)))
                        {
                            const int l_posY = l_window->Size.y - m_mousePosition.y;
                            SendWinAPIMessage(reinterpret_cast<HWND>(l_window->Handle), WM_MOUSEMOVE, NULL, MAKELPARAM(m_mousePosition.x, l_posY));
                            SendWinAPIMessage(reinterpret_cast<HWND>(l_window->Handle), WM_MOUSEWHEEL, MAKEWPARAM(NULL, m_event.data.scroll.ydelta * WHEEL_DELTA), MAKELPARAM(m_mousePosition.x, l_posY));
                        }
                    }
                } break;
            }
        }

        while(ms_vrOverlay->PollNextOverlayEvent(m_overlayControl, &m_event, sizeof(vr::VREvent_t)))
        {
            switch(m_event.eventType)
            {
                case vr::VREvent_MouseMove:
                    m_guiSystem->ProcessMove(static_cast<unsigned int>(m_event.data.mouse.x), static_cast<unsigned int>(m_event.data.mouse.y));
                    break;

                case vr::VREvent_MouseButtonDown:
                {
                    if(m_event.data.mouse.button == vr::VRMouseButton_Left)
                    {
                        m_guiSystem->ProcessClick(GuiClick::GC_Left, GuiClickState::GCS_Press, static_cast<unsigned int>(m_event.data.mouse.x), static_cast<unsigned int>(m_event.data.mouse.y));
                    }
                } break;
            }
        }

        if(m_activeMove)
        {
            const glm::quat l_rot = glm::rotate(VRTransform::GetLeftHandRotation(), -g_PiHalf, g_AxisX);
            m_transform->SetRotation(l_rot);
            m_transform->SetPosition(VRTransform::GetLeftHandPosition());
        }
        if(m_activeResize)
        {
            m_overlayWidth = (glm::distance(VRTransform::GetRightHandPosition(), m_transform->GetPosition()) * 2.f);
            m_transformControl->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.072f, 0.f, 0.f));
            ms_vrOverlay->SetOverlayWidthInMeters(m_overlay, m_overlayWidth);
        }
        m_transform->Update();
        ms_vrOverlay->SetOverlayTransformAbsolute(m_overlay, vr::TrackingUniverseRawAndUncalibrated, &m_transform->GetMatrixVR());

        m_transformControl->Update(m_transform);
        if(m_activeDashboard)
        {
            m_guiSystem->Update();

            ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayControl, vr::TrackingUniverseRawAndUncalibrated, &m_transformControl->GetMatrixVR());
            ms_vrOverlay->SetOverlayTexture(m_overlayControl, &m_textureControls);
        }

        if(m_windowGrabber->IsStale())
        {
            // Window was resized or destroyed
            m_windowGrabber->StopCapture();
            m_windowGrabber->UpdateWindows();
            m_windowIndex = 0U;
            StartCapture();
        }
        m_windowGrabber->Update();
        if(m_texture.handle) ms_vrOverlay->SetOverlayTexture(m_overlay, &m_texture);
    }
}

void WidgetWindowCapture::OnButtonPress(unsigned char f_hand, uint32_t f_button)
{
    Widget::OnButtonPress(f_hand, f_button);

    if(m_valid)
    {
        switch(f_hand)
        {
            case VRHandIndex::VRHI_Left:
            {
                switch(f_button)
                {
                    case vr::k_EButton_SteamVR_Trigger:
                    {
                        if(m_visible && !m_activeDashboard && !m_activePin)
                        {
                            const ULONGLONG l_tick = GetTickCount64();
                            if((l_tick - m_lastLeftTriggerTick) < 500U)
                            {
                                if(!m_activeMove)
                                {
                                    m_activeMove = (glm::distance(VRTransform::GetLeftHandPosition(), m_transform->GetPosition()) < (m_overlayWidth * 0.2f));
                                }
                                else m_activeMove = false;
                            }
                            m_lastLeftTriggerTick = l_tick;
                        }
                    } break;
                }
            } break;

            case VRHandIndex::VRHI_Right:
            {
                if(m_activeMove && (f_button == vr::k_EButton_SteamVR_Trigger))
                {
                    const ULONGLONG l_tick = GetTickCount64();
                    if((l_tick - m_lastRightTriggerTick) < 500U)
                    {
                        m_activeResize = (glm::distance(VRTransform::GetRightHandPosition(), m_transform->GetPosition()) <= (m_overlayWidth * 0.5f));
                    }
                    m_lastRightTriggerTick = l_tick;
                }
            } break;
        }
    }
}

void WidgetWindowCapture::OnButtonRelease(unsigned char f_hand, uint32_t f_button)
{
    Widget::OnButtonRelease(f_hand, f_button);

    if(m_valid && m_visible)
    {
        if((f_hand == VRHandIndex::VRHI_Right) && (f_button == vr::k_EButton_SteamVR_Trigger))
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

        ms_vrOverlay->ShowOverlay(m_overlayControl);
    }
}
void WidgetWindowCapture::OnDashboardClose()
{
    Widget::OnDashboardClose();

    if(m_valid && m_visible) ms_vrOverlay->HideOverlay(m_overlayControl);
}

void WidgetWindowCapture::StartCapture()
{
    if(m_windowGrabber->StartCapture(m_windowIndex))
    {
        m_texture.handle = m_windowGrabber->GetTextureHandle();
        const auto *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
        if(l_window)
        {
            const vr::HmdVector2_t l_scale = { static_cast<float>(l_window->Size.x), static_cast<float>(l_window->Size.y) };
            ms_vrOverlay->SetOverlayMouseScale(m_overlay, &l_scale);

            m_guiTextWindow->Set(l_window->Name);
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
                const size_t l_windowsCount = m_windowGrabber->GetWindowsCount();
                if(l_windowsCount > 0U)
                {
                    m_windowIndex += (l_windowsCount - 1U);
                    m_windowIndex %= l_windowsCount;

                    m_windowGrabber->StopCapture();
                    StartCapture();
                }
            } break;

            case CEI_Next:
            {
                const size_t l_windowsCount = m_windowGrabber->GetWindowsCount();
                if(l_windowsCount > 0U)
                {
                    m_windowIndex += 1U;
                    m_windowIndex %= l_windowsCount;

                    m_windowGrabber->StopCapture();
                    StartCapture();
                }
            } break;

            case CEI_Update:
            {
                m_windowGrabber->StopCapture();
                m_windowGrabber->UpdateWindows();
                m_windowIndex = 0U;
                StartCapture();
            } break;
            case CEI_FPS:
            {
                m_fpsMode += 1U;
                m_fpsMode %= FM_Count;

                m_guiImages[CEI_FPS]->SetUV(g_GuiButtonFpsUV[m_fpsMode], g_GuiButtonsDefaultSize);
                m_windowGrabber->SetDelay(g_CaptureDelays[m_fpsMode]);
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

    WindowGrabber::RemoveStaticResources();
}
