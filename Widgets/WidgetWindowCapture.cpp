#include "stdafx.h"

#include "Widgets/WidgetWindowCapture.h"
#include "Utils/Transformation.h"
#include "Utils/WindowGrabber.h"

#include "Core/GlobalSettings.h"
#include "Core/VRTransform.h"
#include "Utils/GlobalStructures.h"
#include "Utils/Utils.h"

extern const float g_Pi;
extern const glm::ivec2 g_EmptyIVector2;
extern const glm::vec3 g_AxisX;
extern const glm::vec3 g_AxisZN;
extern const sf::Color g_ClearColor;
extern const unsigned char g_DummyTextureData[];

sf::Texture* WidgetWindowCapture::ms_iconsAtlas = nullptr;
vr::Texture_t WidgetWindowCapture::ms_textureControls = { 0 };

const size_t g_CaptureDelays[3U] = {
    66U, 33U, 16U
};
const vr::VRTextureBounds_t g_CaptureDelaysBounds[3U] = {
    { 0.5f, 0.75f, 0.75f, 0.5f },
    { 0.75f, 0.75f, 1.f, 0.5f },
    { 0.f, 0.5f, 0.25f, 0.25f }
};

WidgetWindowCapture::WidgetWindowCapture()
{
    for(size_t i = 0U; i < ControlIndex_Max; i++)
    {
        m_overlayControlHandles[i] = vr::k_ulOverlayHandleInvalid;
        m_transformControls[i] = nullptr;
    }
    m_overlayEvent = { 0 };
    m_windowGrabber = nullptr;
    m_windowIndex = std::numeric_limits<size_t>::max();
    m_lastLeftTriggerTick = 0U;
    m_lastRightTriggerTick = 0U;
    m_closed = false;
    m_activeDashboard = false;
    m_activeMove = false;
    m_activeResize = false;
    m_activePin = false;
    m_overlayWidth = 0.f;
    m_windowSize = g_EmptyIVector2;
    m_mousePosition = g_EmptyIVector2;
    m_fpsMode = FpsMode_15;
}
WidgetWindowCapture::~WidgetWindowCapture()
{
    Cleanup();
}

bool WidgetWindowCapture::Create()
{
    if(!m_valid)
    {
        m_visible = true; // Visible by default
        m_overlayWidth = 0.5f;

        if(!ms_iconsAtlas)
        {
            ms_iconsAtlas = new sf::Texture();
            if(!ms_iconsAtlas->loadFromFile("icons/atlas_capture.png")) ms_iconsAtlas->loadFromMemory(g_DummyTextureData, 16U);

            ms_textureControls.eType = vr::TextureType_OpenGL;
            ms_textureControls.eColorSpace = vr::ColorSpace_Gamma;
            ms_textureControls.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(ms_iconsAtlas->getNativeHandle()));
        }

        for(size_t i = 0U; i < ControlIndex_Max; i++) m_transformControls[i] = new Transformation();

        std::string l_overlayKeyPart("ovrw.capture_");
        l_overlayKeyPart.append(std::to_string(reinterpret_cast<size_t>(this)));

        std::string l_overlayKeyFull(l_overlayKeyPart);
        l_overlayKeyFull.append(".main");
        if(ms_vrOverlay->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Capture - Main", &m_overlayHandle) == vr::VROverlayError_None)
        {
            ms_vrOverlay->SetOverlayWidthInMeters(m_overlayHandle, m_overlayWidth);
            ms_vrOverlay->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_SortWithNonSceneOverlays, true);
            ms_vrOverlay->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_ProtectedContent, true);
            ms_vrOverlay->SetOverlayInputMethod(m_overlayHandle, vr::VROverlayInputMethod_Mouse);
            ms_vrOverlay->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_ShowTouchPadScrollWheel, true);
            ms_vrOverlay->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_SendVRDiscreteScrollEvents, true);

            // Change UV
            vr::VRTextureBounds_t l_bounds = { 0.f, 1.f, 1.f, 0.f };
            ms_vrOverlay->SetOverlayTextureBounds(m_overlayHandle, &l_bounds);

            // Create overlay in front of camera
            glm::vec3 l_initPos(VRTransform::GetHmdPosition());
            l_initPos += (VRTransform::GetHmdRotation()*g_AxisZN)*0.5f;
            m_transform->SetPosition(l_initPos);

            glm::quat l_initRot;
            GetRotationToPoint(VRTransform::GetHmdPosition(), l_initPos, VRTransform::GetHmdRotation(), l_initRot);
            m_transform->SetRotation(l_initRot);
            m_transform->Update();
            ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseRawAndUncalibrated, &m_transform->GetMatrixVR());

            m_vrTexture.eType = vr::TextureType_OpenGL;
            m_vrTexture.eColorSpace = vr::ColorSpace_Gamma;
            m_vrTexture.handle = nullptr;

            l_overlayKeyFull.assign(l_overlayKeyPart);
            l_overlayKeyFull.append(".pin");
            if(ms_vrOverlay->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Capture - Pin", &m_overlayControlHandles[ControlIndex_PinUnpin]) == vr::VROverlayError_None)
            {
                m_transformControls[ControlIndex_PinUnpin]->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, 0.06f, 0.f));
                l_bounds = { 0.f, 1.f, 0.25f, 0.75f };
                ms_vrOverlay->SetOverlayTextureBounds(m_overlayControlHandles[ControlIndex_PinUnpin], &l_bounds);

                l_overlayKeyFull.assign(l_overlayKeyPart);
                l_overlayKeyFull.append(".close");
                if(ms_vrOverlay->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Capture - Close", &m_overlayControlHandles[ControlIndex_Close]) == vr::VROverlayError_None)
                {
                    m_transformControls[ControlIndex_Close]->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.095f, 0.06f, 0.f));
                    l_bounds = { 0.5f, 1.f, 0.75f, 0.75f };
                    ms_vrOverlay->SetOverlayTextureBounds(m_overlayControlHandles[ControlIndex_Close], &l_bounds);

                    l_overlayKeyFull.assign(l_overlayKeyPart);
                    l_overlayKeyFull.append(".previous");
                    if(ms_vrOverlay->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Capture - Previous", &m_overlayControlHandles[ControlIndex_Previous]) == vr::VROverlayError_None)
                    {
                        m_transformControls[ControlIndex_Previous]->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, 0.f, 0.f));
                        l_bounds = { 0.75f, 1.f, 1.f, 0.75f };
                        ms_vrOverlay->SetOverlayTextureBounds(m_overlayControlHandles[ControlIndex_Previous], &l_bounds);

                        l_overlayKeyFull.assign(l_overlayKeyPart);
                        l_overlayKeyFull.append(".next");
                        if(ms_vrOverlay->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Capture - Next", &m_overlayControlHandles[ControlIndex_Next]) == vr::VROverlayError_None)
                        {
                            m_transformControls[ControlIndex_Next]->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.095f, 0.f, 0.f));
                            l_bounds = { 0.f, 0.75f, 0.25f, 0.5f };
                            ms_vrOverlay->SetOverlayTextureBounds(m_overlayControlHandles[ControlIndex_Next], &l_bounds);

                            l_overlayKeyFull.assign(l_overlayKeyPart);
                            l_overlayKeyFull.append(".update");
                            if(ms_vrOverlay->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Capture - Update", &m_overlayControlHandles[ControlIndex_Update]) == vr::VROverlayError_None)
                            {
                                m_transformControls[ControlIndex_Update]->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, -0.06f, 0.f));
                                l_bounds = { 0.25f, 0.75f, 0.5f, 0.5f };
                                ms_vrOverlay->SetOverlayTextureBounds(m_overlayControlHandles[ControlIndex_Update], &l_bounds);

                                l_overlayKeyFull.assign(l_overlayKeyPart);
                                l_overlayKeyFull.append(".fps");
                                if(ms_vrOverlay->CreateOverlay(l_overlayKeyFull.c_str(), "OpenVR Widgets - Capture - FPS", &m_overlayControlHandles[ControlIndex_ChangeFps]) == vr::VROverlayError_None)
                                {
                                    m_transformControls[ControlIndex_ChangeFps]->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.095f, -0.06f, 0.f));
                                    ms_vrOverlay->SetOverlayTextureBounds(m_overlayControlHandles[ControlIndex_ChangeFps], &g_CaptureDelaysBounds[FpsMode_15]);

                                    // Apply same properties
                                    for(size_t i = 0U; i < ControlIndex_Max; i++)
                                    {
                                        ms_vrOverlay->SetOverlayWidthInMeters(m_overlayControlHandles[i], 0.05f);
                                        ms_vrOverlay->SetOverlayInputMethod(m_overlayControlHandles[i], vr::VROverlayInputMethod::VROverlayInputMethod_Mouse);
                                        m_transformControls[i]->Update(m_transform);
                                        ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayControlHandles[i], vr::TrackingUniverseRawAndUncalibrated, &m_transformControls[i]->GetMatrixVR());
                                        ms_vrOverlay->SetOverlayTexture(m_overlayControlHandles[i], &ms_textureControls);
                                    }

                                    // Create window grabber and start capture
                                    m_windowGrabber = new WindowGrabber();
                                    m_windowGrabber->UpdateWindows();
                                    m_windowIndex = 0U;
                                    InternalStartCapture();

                                    // Show main overlay
                                    ms_vrOverlay->ShowOverlay(m_overlayHandle);

                                    m_valid = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if(!m_valid) Cleanup();

    return m_valid;
}

void WidgetWindowCapture::Destroy()
{
    Cleanup();
}

void WidgetWindowCapture::Cleanup()
{
    if(m_overlayHandle != vr::k_ulOverlayHandleInvalid)
    {
        ms_vrOverlay->HideOverlay(m_overlayHandle);
        ms_vrOverlay->ClearOverlayTexture(m_overlayHandle);
        ms_vrOverlay->DestroyOverlay(m_overlayHandle);
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    }

    for(size_t i = 0U; i < ControlIndex_Max; i++)
    {
        if(m_overlayControlHandles[i] != vr::k_ulOverlayHandleInvalid)
        {
            ms_vrOverlay->HideOverlay(m_overlayControlHandles[i]);
            ms_vrOverlay->ClearOverlayTexture(m_overlayControlHandles[i]);
            ms_vrOverlay->DestroyOverlay(m_overlayControlHandles[i]);
            m_overlayControlHandles[i] = vr::k_ulOverlayHandleInvalid;
        }

        delete m_transformControls[i];
        m_transformControls[i] = nullptr;
    }

    delete m_windowGrabber;
    m_windowGrabber = nullptr;

    m_valid = false;
}

void WidgetWindowCapture::Update()
{
    if(m_valid && m_visible)
    {
        // Poll overlays interaction
        while(ms_vrOverlay->PollNextOverlayEvent(m_overlayHandle, &m_overlayEvent, sizeof(vr::VREvent_t)))
        {
            switch(m_overlayEvent.eventType)
            {
                case vr::VREvent_MouseMove:
                {
                    if(m_activeDashboard)
                    {
                        const auto *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
                        if(l_window)
                        {
                            if(IsWindow(reinterpret_cast<HWND>(l_window->Handle)))
                            {
                                m_mousePosition.x = static_cast<int>(m_overlayEvent.data.mouse.x);
                                m_mousePosition.y = l_window->Size.y - static_cast<int>(m_overlayEvent.data.mouse.y);
                            }
                        }
                    }
                } break;

                case vr::VREvent_MouseButtonDown:
                {
                    if(m_activeDashboard)
                    {
                        const auto *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
                        if(l_window)
                        {
                            if(IsWindow(reinterpret_cast<HWND>(l_window->Handle)))
                            {
                                DWORD l_buttonData[3] = { 0 };
                                switch(m_overlayEvent.data.mouse.button)
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

                                SendWinAPIMessage(reinterpret_cast<HWND>(l_window->Handle), WM_MOUSEMOVE, NULL, MAKELPARAM(m_mousePosition.x, m_mousePosition.y));
                                SendWinAPIMessage(reinterpret_cast<HWND>(l_window->Handle), l_buttonData[1], l_buttonData[0], MAKELPARAM(m_mousePosition.x, m_mousePosition.y));
                                SendWinAPIMessage(reinterpret_cast<HWND>(l_window->Handle), l_buttonData[2], NULL, MAKELPARAM(m_mousePosition.x, m_mousePosition.y));
                            }
                        }
                    }
                } break;

                case vr::VREvent_ScrollDiscrete:
                {
                    if(m_activeDashboard)
                    {
                        const auto *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
                        if(l_window)
                        {
                            if(IsWindow(reinterpret_cast<HWND>(l_window->Handle)))
                            {
                                SendWinAPIMessage(reinterpret_cast<HWND>(l_window->Handle), WM_MOUSEMOVE, NULL, MAKELPARAM(m_mousePosition.x, m_mousePosition.y));
                                SendWinAPIMessage(reinterpret_cast<HWND>(l_window->Handle), WM_MOUSEWHEEL, MAKEWPARAM(NULL, m_overlayEvent.data.scroll.ydelta * WHEEL_DELTA), MAKELPARAM(m_mousePosition.x, m_mousePosition.y));
                            }
                        }
                    }
                } break;
            }
        }

        while(ms_vrOverlay->PollNextOverlayEvent(m_overlayControlHandles[ControlIndex_PinUnpin], &m_overlayEvent, sizeof(vr::VREvent_t)))
        {
            switch(m_overlayEvent.eventType)
            {
                case vr::VREvent_MouseButtonDown:
                {
                    if(m_overlayEvent.data.mouse.button == vr::VRMouseButton_Left)
                    {
                        m_activePin = !m_activePin;

                        vr::VRTextureBounds_t l_bounds;
                        if(m_activePin) l_bounds = { 0.25f, 1.f, 0.5f, 0.75f };
                        else l_bounds = { 0.f, 1.f, 0.25f, 0.75f };
                        ms_vrOverlay->SetOverlayTextureBounds(m_overlayControlHandles[ControlIndex_PinUnpin], &l_bounds);
                    }
                } break;
            }
        }

        while(ms_vrOverlay->PollNextOverlayEvent(m_overlayControlHandles[ControlIndex_Close], &m_overlayEvent, sizeof(vr::VREvent_t)))
        {
            switch(m_overlayEvent.eventType)
            {
                case vr::VREvent_MouseButtonDown:
                {
                    if(m_overlayEvent.data.mouse.button == vr::VRMouseButton_Left)
                    {
                        m_closed = true;
                    }
                } break;
            }
        }

        while(ms_vrOverlay->PollNextOverlayEvent(m_overlayControlHandles[ControlIndex_Previous], &m_overlayEvent, sizeof(vr::VREvent_t)))
        {
            switch(m_overlayEvent.eventType)
            {
                case vr::VREvent_MouseButtonDown:
                {
                    if(m_overlayEvent.data.mouse.button == vr::VRMouseButton_Left)
                    {
                        size_t l_windowsCount = m_windowGrabber->GetWindowsCount();
                        if(l_windowsCount > 0U)
                        {
                            m_windowIndex += (l_windowsCount - 1U);
                            m_windowIndex %= l_windowsCount;

                            m_windowGrabber->StopCapture();
                            InternalStartCapture();
                        }
                    }
                } break;
            }
        }

        while(ms_vrOverlay->PollNextOverlayEvent(m_overlayControlHandles[ControlIndex_Next], &m_overlayEvent, sizeof(vr::VREvent_t)))
        {
            switch(m_overlayEvent.eventType)
            {
                case vr::VREvent_MouseButtonDown:
                {
                    if(m_overlayEvent.data.mouse.button == vr::VRMouseButton_Left)
                    {
                        size_t l_windowsCount = m_windowGrabber->GetWindowsCount();
                        if(l_windowsCount > 0U)
                        {
                            m_windowIndex += 1U;
                            m_windowIndex %= l_windowsCount;

                            m_windowGrabber->StopCapture();
                            InternalStartCapture();
                        }
                    }
                } break;
            }
        }

        while(ms_vrOverlay->PollNextOverlayEvent(m_overlayControlHandles[ControlIndex_Update], &m_overlayEvent, sizeof(vr::VREvent_t)))
        {
            switch(m_overlayEvent.eventType)
            {
                case vr::VREvent_MouseButtonDown:
                {
                    if(m_overlayEvent.data.mouse.button == vr::VRMouseButton_Left)
                    {
                        m_windowGrabber->StopCapture();
                        m_windowGrabber->UpdateWindows();
                        m_windowIndex = 0U;
                        InternalStartCapture();
                    }
                } break;
            }
        }

        while(ms_vrOverlay->PollNextOverlayEvent(m_overlayControlHandles[ControlIndex_ChangeFps], &m_overlayEvent, sizeof(vr::VREvent_t)))
        {
            switch(m_overlayEvent.eventType)
            {
                case vr::VREvent_MouseButtonDown:
                {
                    if(m_overlayEvent.data.mouse.button == vr::VRMouseButton_Left)
                    {
                        m_fpsMode += 1U;
                        m_fpsMode %= FpsMode_Max;

                        m_windowGrabber->SetDelay(g_CaptureDelays[m_fpsMode]);
                        ms_vrOverlay->SetOverlayTextureBounds(m_overlayControlHandles[ControlIndex_ChangeFps], &g_CaptureDelaysBounds[m_fpsMode]);
                    }
                } break;
            }
        }

        if(m_activeMove)
        {
            glm::quat l_rot = glm::rotate(VRTransform::GetLeftHandRotation(), -g_Pi*0.5f, g_AxisX);
            m_transform->SetRotation(l_rot);
            m_transform->SetPosition(VRTransform::GetLeftHandPosition());
        }
        if(m_activeResize)
        {
            m_overlayWidth = (glm::distance(VRTransform::GetRightHandPosition(), m_transform->GetPosition()) * 2.f);
            ms_vrOverlay->SetOverlayWidthInMeters(m_overlayHandle, m_overlayWidth);

            m_transformControls[ControlIndex_PinUnpin]->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, 0.06f, 0.f));
            m_transformControls[ControlIndex_Close]->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.095f, 0.06f, 0.f));
            m_transformControls[ControlIndex_Previous]->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, 0.f, 0.f));
            m_transformControls[ControlIndex_Next]->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.095f, 0.f, 0.f));
            m_transformControls[ControlIndex_Update]->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, -0.06f, 0.f));
            m_transformControls[ControlIndex_ChangeFps]->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.095f, -0.06f, 0.f));
        }

        if(m_windowGrabber->IsStale())
        {
            // Window was resized or destroyed
            m_windowGrabber->StopCapture();
            m_windowGrabber->UpdateWindows();
            m_windowIndex = 0U;
            InternalStartCapture();
        }
        m_windowGrabber->Update();
        if(m_vrTexture.handle) ms_vrOverlay->SetOverlayTexture(m_overlayHandle, &m_vrTexture);

        m_transform->Update();
        ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseRawAndUncalibrated, &m_transform->GetMatrixVR());

        for(size_t i = 0U; i < ControlIndex_Max; i++)
        {
            m_transformControls[i]->Update(m_transform);
            if(m_activeDashboard)
            {
                ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayControlHandles[i], vr::TrackingUniverseRawAndUncalibrated, &m_transformControls[i]->GetMatrixVR());
                ms_vrOverlay->SetOverlayTexture(m_overlayControlHandles[i], &ms_textureControls);
            }
        }
    }
}

void WidgetWindowCapture::OnButtonPress(unsigned char f_hand, uint32_t f_button)
{
    if(m_valid)
    {
        switch(f_hand)
        {
            case VRHand::VRHand_Left:
            {
                switch(f_button)
                {
                    case vr::k_EButton_SteamVR_Trigger:
                    {
                        if(m_visible && !m_activeDashboard && !m_activePin)
                        {
                            ULONGLONG l_tick = GetTickCount64();
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

            case VRHand::VRHand_Right:
            {
                if(m_activeMove && (f_button == vr::k_EButton_SteamVR_Trigger))
                {
                    ULONGLONG l_tick = GetTickCount64();
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

bool WidgetWindowCapture::CloseRequested() const
{
    return m_closed;
}

void WidgetWindowCapture::OnButtonRelease(unsigned char f_hand, uint32_t f_button)
{
    if(m_valid && m_visible)
    {
        if((f_hand == VRHand::VRHand_Right) && (f_button == vr::k_EButton_SteamVR_Trigger))
        {
            if(m_activeResize) m_activeResize = false;
        }
    }
}

void WidgetWindowCapture::OnDashboardOpen()
{
    if(m_valid)
    {
        m_activeDashboard = true;
        if(m_visible)
        {
            m_activeMove = false;
            m_activeResize = false;

            for(size_t i = 0U; i < ControlIndex_Max; i++) ms_vrOverlay->ShowOverlay(m_overlayControlHandles[i]);
        }
    }
}
void WidgetWindowCapture::OnDashboardClose()
{
    if(m_valid)
    {
        m_activeDashboard = false;
        if(m_visible)
        {
            for(size_t i = 0U; i < ControlIndex_Max; i++) ms_vrOverlay->HideOverlay(m_overlayControlHandles[i]);
        }
    }
}

void WidgetWindowCapture::RemoveIconsAtlas()
{
    if(ms_iconsAtlas)
    {
        delete ms_iconsAtlas;
        ms_iconsAtlas = nullptr;
        ms_textureControls = { 0 };
    }
}

// Internal methods
void WidgetWindowCapture::InternalStartCapture()
{
    if(m_windowGrabber->StartCapture(m_windowIndex))
    {
        m_vrTexture.handle = m_windowGrabber->GetTextureHandle();
        const auto *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
        if(l_window)
        {
            vr::HmdVector2_t l_scale = { static_cast<float>(l_window->Size.x), static_cast<float>(l_window->Size.y) };
            ms_vrOverlay->SetOverlayMouseScale(m_overlayHandle, &l_scale);
        }
    }
    else m_vrTexture.handle = nullptr;
}
