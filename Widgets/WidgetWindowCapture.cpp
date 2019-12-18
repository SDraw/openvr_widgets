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

WidgetWindowCapture::WidgetWindowCapture()
{
    m_overlayNextHandle = vr::k_ulOverlayHandleInvalid;
    m_overlayPrevHandle = vr::k_ulOverlayHandleInvalid;
    m_overlayUpdateHandle = vr::k_ulOverlayHandleInvalid;
    m_overlayPinHandle = vr::k_ulOverlayHandleInvalid;
    m_overlayEvent = { 0 };
    m_windowGrabber = nullptr;
    m_windowIndex = std::numeric_limits<size_t>::max();
    m_lastLeftGripTick = 0U;
    m_lastLeftTriggerTick = 0U;
    m_lastRightTriggerTick = 0U;
    m_activeFirstTime = true;
    m_activeDashboard = false;
    m_activeMove = false;
    m_activeResize = false;
    m_activePin = false;
    m_overlayWidth = 0.f;
    m_windowSize = g_EmptyIVector2;
    m_mousePosition = g_EmptyIVector2;
    m_pinButtonTransform = nullptr;
    m_nextButtonTransform = nullptr;
    m_prevButtonTransform = nullptr;
    m_updButtonTransform = nullptr;
}
WidgetWindowCapture::~WidgetWindowCapture()
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
    if(m_overlayPinHandle != vr::k_ulOverlayHandleInvalid)
    {
        ms_vrOverlay->HideOverlay(m_overlayPinHandle);
        ms_vrOverlay->ClearOverlayTexture(m_overlayPinHandle);
        ms_vrOverlay->DestroyOverlay(m_overlayPinHandle);
        m_overlayPinHandle = vr::k_ulOverlayHandleInvalid;
    }
    if(m_overlayNextHandle != vr::k_ulOverlayHandleInvalid)
    {
        ms_vrOverlay->HideOverlay(m_overlayNextHandle);
        ms_vrOverlay->ClearOverlayTexture(m_overlayNextHandle);
        ms_vrOverlay->DestroyOverlay(m_overlayNextHandle);
        m_overlayNextHandle = vr::k_ulOverlayHandleInvalid;
    }
    if(m_overlayPrevHandle != vr::k_ulOverlayHandleInvalid)
    {
        ms_vrOverlay->HideOverlay(m_overlayPrevHandle);
        ms_vrOverlay->ClearOverlayTexture(m_overlayPrevHandle);
        ms_vrOverlay->DestroyOverlay(m_overlayPrevHandle);
        m_overlayPrevHandle = vr::k_ulOverlayHandleInvalid;
    }
    if(m_overlayUpdateHandle != vr::k_ulOverlayHandleInvalid)
    {
        ms_vrOverlay->HideOverlay(m_overlayUpdateHandle);
        ms_vrOverlay->ClearOverlayTexture(m_overlayUpdateHandle);
        ms_vrOverlay->DestroyOverlay(m_overlayUpdateHandle);
        m_overlayUpdateHandle = vr::k_ulOverlayHandleInvalid;
    }

    delete m_pinButtonTransform;
    delete m_nextButtonTransform;
    delete m_prevButtonTransform;
    delete m_updButtonTransform;

    delete m_windowGrabber;

    m_valid = false;
}

bool WidgetWindowCapture::Create()
{
    if(!m_valid)
    {
        m_overlayWidth = 0.5f;
        m_pinButtonTransform = new Transformation();
        m_nextButtonTransform = new Transformation();
        m_prevButtonTransform = new Transformation();
        m_updButtonTransform = new Transformation();

        if(ms_vrOverlay->CreateOverlay("ovrw.capture.main", "OpenVR Widget - Capture - Main", &m_overlayHandle) == vr::VROverlayError_None)
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

            m_vrTexture.eType = vr::TextureType_OpenGL;
            m_vrTexture.eColorSpace = vr::ColorSpace_Gamma;
            m_vrTexture.handle = nullptr;

            std::string l_iconPath;
            if(ms_vrOverlay->CreateOverlay("ovrw.capture.pin", "OpenVR Widget - Capture - Pin", &m_overlayPinHandle) == vr::VROverlayError_None)
            {
                l_iconPath.assign("\\icons\\unpinned.png");
                l_iconPath.insert(0U, GlobalSettings::GetDirectory());

                m_pinButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, 0.09f, 0.f));
                m_pinButtonTransform->Update(m_transform);

                ms_vrOverlay->SetOverlayWidthInMeters(m_overlayPinHandle, 0.05f);
                ms_vrOverlay->SetOverlayFromFile(m_overlayPinHandle, l_iconPath.c_str());
                ms_vrOverlay->SetOverlayInputMethod(m_overlayPinHandle, vr::VROverlayInputMethod::VROverlayInputMethod_Mouse);
                ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayPinHandle, vr::TrackingUniverseRawAndUncalibrated, &m_pinButtonTransform->GetMatrixVR());

                if(ms_vrOverlay->CreateOverlay("ovrw.capture.next", "OpenVR Widget - Capture - Next", &m_overlayNextHandle) == vr::VROverlayError_None)
                {
                    l_iconPath.assign("\\icons\\next.png");
                    l_iconPath.insert(0U, GlobalSettings::GetDirectory());

                    m_nextButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, 0.03f, 0.f));
                    m_nextButtonTransform->Update(m_transform);

                    ms_vrOverlay->SetOverlayWidthInMeters(m_overlayNextHandle, 0.05f);
                    ms_vrOverlay->SetOverlayFromFile(m_overlayNextHandle, l_iconPath.c_str());
                    ms_vrOverlay->SetOverlayInputMethod(m_overlayNextHandle, vr::VROverlayInputMethod::VROverlayInputMethod_Mouse);
                    ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayNextHandle, vr::TrackingUniverseRawAndUncalibrated, &m_nextButtonTransform->GetMatrixVR());

                    if(ms_vrOverlay->CreateOverlay("ovrw.capture.previous", "OpenVR Widget - Capture - Previous", &m_overlayPrevHandle) == vr::VROverlayError_None)
                    {
                        l_iconPath.assign("\\icons\\prev.png");
                        l_iconPath.insert(0U, GlobalSettings::GetDirectory());

                        m_prevButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, -0.03f, 0.f));
                        m_prevButtonTransform->Update(m_transform);

                        ms_vrOverlay->SetOverlayWidthInMeters(m_overlayPrevHandle, 0.05f);
                        ms_vrOverlay->SetOverlayFromFile(m_overlayPrevHandle, l_iconPath.c_str());
                        ms_vrOverlay->SetOverlayInputMethod(m_overlayPrevHandle, vr::VROverlayInputMethod::VROverlayInputMethod_Mouse);
                        ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayPrevHandle, vr::TrackingUniverseRawAndUncalibrated, &m_prevButtonTransform->GetMatrixVR());

                        if(ms_vrOverlay->CreateOverlay("ovrw.capture.update", "OpenVR Widget - Capture - Update", &m_overlayUpdateHandle) == vr::VROverlayError_None)
                        {
                            l_iconPath.assign("\\icons\\upd.png");
                            l_iconPath.insert(0U, GlobalSettings::GetDirectory());

                            m_updButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, -0.09f, 0.f));
                            m_updButtonTransform->Update(m_transform);

                            ms_vrOverlay->SetOverlayWidthInMeters(m_overlayUpdateHandle, 0.05f);
                            ms_vrOverlay->SetOverlayFromFile(m_overlayUpdateHandle, l_iconPath.c_str());
                            ms_vrOverlay->SetOverlayInputMethod(m_overlayUpdateHandle, vr::VROverlayInputMethod::VROverlayInputMethod_Mouse);
                            ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayUpdateHandle, vr::TrackingUniverseRawAndUncalibrated, &m_updButtonTransform->GetMatrixVR());

                            // Create window grabber
                            m_windowGrabber = new WindowGrabber();
                            m_windowGrabber->UpdateWindows();
                            m_windowIndex = 0U;

                            m_valid = true;
                        }
                    }
                }
            }
        }
    }
    return m_valid;
}
void WidgetWindowCapture::Destroy()
{
    Cleanup();
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

        while(ms_vrOverlay->PollNextOverlayEvent(m_overlayPinHandle, &m_overlayEvent, sizeof(vr::VREvent_t)))
        {
            switch(m_overlayEvent.eventType)
            {
                case vr::VREvent_MouseButtonDown:
                {
                    if(m_overlayEvent.data.mouse.button == vr::EVRMouseButton::VRMouseButton_Left)
                    {
                        m_activePin = !m_activePin;

                        std::string l_iconPath(m_activePin ? "\\icons\\pinned.png" : "\\icons\\unpinned.png");
                        l_iconPath.insert(0U, GlobalSettings::GetDirectory());
                        ms_vrOverlay->ClearOverlayTexture(m_overlayPinHandle);
                        ms_vrOverlay->SetOverlayFromFile(m_overlayPinHandle, l_iconPath.c_str());
                    }
                } break;
            }
        }

        while(ms_vrOverlay->PollNextOverlayEvent(m_overlayNextHandle, &m_overlayEvent, sizeof(vr::VREvent_t)))
        {
            switch(m_overlayEvent.eventType)
            {
                case vr::VREvent_MouseButtonDown:
                {
                    if(m_overlayEvent.data.mouse.button == vr::EVRMouseButton::VRMouseButton_Left)
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

        while(ms_vrOverlay->PollNextOverlayEvent(m_overlayPrevHandle, &m_overlayEvent, sizeof(vr::VREvent_t)))
        {
            switch(m_overlayEvent.eventType)
            {
                case vr::VREvent_MouseButtonDown:
                {
                    if(m_overlayEvent.data.mouse.button == vr::EVRMouseButton::VRMouseButton_Left)
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
        while(ms_vrOverlay->PollNextOverlayEvent(m_overlayUpdateHandle, &m_overlayEvent, sizeof(vr::VREvent_t)))
        {
            switch(m_overlayEvent.eventType)
            {
                case vr::VREvent_MouseButtonDown:
                {
                    if(m_overlayEvent.data.mouse.button == vr::EVRMouseButton::VRMouseButton_Left)
                    {
                        m_windowGrabber->StopCapture();
                        m_windowGrabber->UpdateWindows();
                        m_windowIndex = 0U;
                        InternalStartCapture();
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

            m_pinButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, 0.09f, 0.f));
            m_nextButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, 0.03f, 0.f));
            m_prevButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, -0.03f, 0.f));
            m_updButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, -0.09f, 0.f));
        }

        if(m_windowGrabber->IsStale())
        {
            // Window was resized or destoryed
            m_windowGrabber->StopCapture();
            m_windowGrabber->UpdateWindows();
            m_windowIndex = 0U;
            InternalStartCapture();
        }
        m_windowGrabber->Update();
        if(m_vrTexture.handle) ms_vrOverlay->SetOverlayTexture(m_overlayHandle, &m_vrTexture);

        m_transform->Update();
        ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseRawAndUncalibrated, &m_transform->GetMatrixVR());

        m_pinButtonTransform->Update(m_transform);
        m_nextButtonTransform->Update(m_transform);
        m_prevButtonTransform->Update(m_transform);
        m_updButtonTransform->Update(m_transform);
        if(m_activeDashboard)
        {
            ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayPinHandle, vr::TrackingUniverseRawAndUncalibrated, &m_pinButtonTransform->GetMatrixVR());
            ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayNextHandle, vr::TrackingUniverseRawAndUncalibrated, &m_nextButtonTransform->GetMatrixVR());
            ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayPrevHandle, vr::TrackingUniverseRawAndUncalibrated, &m_prevButtonTransform->GetMatrixVR());
            ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayUpdateHandle, vr::TrackingUniverseRawAndUncalibrated, &m_updButtonTransform->GetMatrixVR());
        }
    }
}

void WidgetWindowCapture::OnButtonPress(unsigned char f_hand, uint32_t f_button)
{
    if(m_valid)
    {
        switch(f_hand)
        {
            case EVRHand::EVRHand_Left:
            {
                switch(f_button)
                {
                    case vr::k_EButton_Grip:
                    {
                        ULONGLONG l_tick = GetTickCount64();
                        if((l_tick - m_lastLeftGripTick) < 300U)
                        {
                            m_visible = !m_visible;
                            if(m_visible)
                            {
                                InternalStartCapture();
                                ms_vrOverlay->ShowOverlay(m_overlayHandle);
                                if(m_activeDashboard)
                                {
                                    ms_vrOverlay->ShowOverlay(m_overlayPinHandle);
                                    ms_vrOverlay->ShowOverlay(m_overlayNextHandle);
                                    ms_vrOverlay->ShowOverlay(m_overlayPrevHandle);
                                    ms_vrOverlay->ShowOverlay(m_overlayUpdateHandle);
                                }
                                if(m_activeFirstTime)
                                {
                                    // Place in front of player 
                                    glm::vec3 l_pos = (VRTransform::GetHmdRotation()*g_AxisZN)*0.5f;
                                    l_pos += VRTransform::GetHmdPosition();
                                    l_pos.y = VRTransform::GetHmdPosition().y;
                                    m_transform->SetPosition(l_pos);

                                    glm::quat l_rot;
                                    GetRotationToPoint(VRTransform::GetHmdPosition(), m_transform->GetPosition(), VRTransform::GetHmdRotation(), l_rot);
                                    m_transform->SetRotation(l_rot);

                                    m_activeFirstTime = false;
                                }
                            }
                            else
                            {
                                ms_vrOverlay->ClearOverlayTexture(m_overlayHandle);
                                m_windowGrabber->StopCapture();
                                m_vrTexture.handle = nullptr;

                                m_activeMove = false;
                                m_activeResize = false;

                                ms_vrOverlay->HideOverlay(m_overlayHandle);
                                if(m_activeDashboard)
                                {
                                    ms_vrOverlay->HideOverlay(m_overlayPinHandle);
                                    ms_vrOverlay->HideOverlay(m_overlayNextHandle);
                                    ms_vrOverlay->HideOverlay(m_overlayPrevHandle);
                                    ms_vrOverlay->HideOverlay(m_overlayUpdateHandle);
                                }
                            }
                        }
                        m_lastLeftGripTick = l_tick;
                    } break;

                    case vr::k_EButton_SteamVR_Trigger:
                    {
                        if(m_visible && !m_activeDashboard && !m_activePin)
                        {
                            ULONGLONG l_tick = GetTickCount64();
                            if((l_tick - m_lastLeftTriggerTick) < 300U)
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

            case EVRHand::EVRHand_Right:
            {
                if(m_activeMove && (f_button == vr::k_EButton_SteamVR_Trigger))
                {
                    ULONGLONG l_tick = GetTickCount64();
                    if((l_tick - m_lastRightTriggerTick) < 300U)
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
    if(m_valid && m_visible)
    {
        if((f_hand == EVRHand::EVRHand_Right) && (f_button == vr::k_EButton_SteamVR_Trigger))
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

            ms_vrOverlay->ShowOverlay(m_overlayPinHandle);
            ms_vrOverlay->ShowOverlay(m_overlayNextHandle);
            ms_vrOverlay->ShowOverlay(m_overlayPrevHandle);
            ms_vrOverlay->ShowOverlay(m_overlayUpdateHandle);
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
            ms_vrOverlay->HideOverlay(m_overlayPinHandle);
            ms_vrOverlay->HideOverlay(m_overlayNextHandle);
            ms_vrOverlay->HideOverlay(m_overlayPrevHandle);
            ms_vrOverlay->HideOverlay(m_overlayUpdateHandle);
        }
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
