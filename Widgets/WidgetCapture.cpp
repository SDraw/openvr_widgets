#include "stdafx.h"

#include "Widgets/WidgetCapture.h"
#include "Core/Config.h"
#include "Core/VRTransform.h"
#include "Core/WindowGrabber.h"
#include "Utils/Transformation.h"
#include "Utils/Utils.h"

extern const float g_Pi;
extern const glm::ivec2 g_EmptyIVector2;
extern const glm::vec3 g_AxisX;
extern const sf::Color g_ClearColor;

WidgetCapture::WidgetCapture()
{
    m_overlayNextHandle = vr::k_ulOverlayHandleInvalid;
    m_overlayPrevHandle = vr::k_ulOverlayHandleInvalid;
    m_overlayUpdateHandle = vr::k_ulOverlayHandleInvalid;
    m_overlayEvent = { 0 };
    m_windowGrabber = nullptr;
    m_windowIndex = std::numeric_limits<size_t>::max();
    m_lastLeftGripTick = 0U;
    m_lastLeftTriggerTick = 0U;
    m_lastRightTriggerTick = 0U;
    m_activeDashboard = false;
    m_activeMove = false;
    m_activeResize = false;
    m_overlayWidth = 0.f;
    m_windowSize = g_EmptyIVector2;
    m_mousePosition = g_EmptyIVector2;
    m_nextButtonTransform = nullptr;
    m_prevButtonTransform = nullptr;
    m_updButtonTransform = nullptr;
}
WidgetCapture::~WidgetCapture()
{
    Cleanup();
}

void WidgetCapture::Cleanup()
{
    if(m_valid)
    {
        ms_vrOverlay->ClearOverlayTexture(m_overlayHandle);
        ms_vrOverlay->HideOverlay(m_overlayHandle);
        ms_vrOverlay->DestroyOverlay(m_overlayHandle);
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;

        ms_vrOverlay->HideOverlay(m_overlayNextHandle);
        ms_vrOverlay->DestroyOverlay(m_overlayNextHandle);
        m_overlayNextHandle = vr::k_ulOverlayHandleInvalid;

        ms_vrOverlay->HideOverlay(m_overlayPrevHandle);
        ms_vrOverlay->DestroyOverlay(m_overlayPrevHandle);
        m_overlayPrevHandle = vr::k_ulOverlayHandleInvalid;

        ms_vrOverlay->HideOverlay(m_overlayUpdateHandle);
        ms_vrOverlay->DestroyOverlay(m_overlayUpdateHandle);
        m_overlayUpdateHandle = vr::k_ulOverlayHandleInvalid;

        delete m_windowGrabber;

        delete m_nextButtonTransform;
        delete m_prevButtonTransform;
        delete m_updButtonTransform;

        m_valid = false;
    }
}

bool WidgetCapture::Create()
{
    if(!m_valid)
    {
        m_overlayWidth = 0.5;
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

            // Init position
            m_transform->SetPosition(glm::vec3(0.f, -1.5f, -2.f));

            m_vrTexture.eType = vr::TextureType_OpenGL;
            m_vrTexture.eColorSpace = vr::ColorSpace_Gamma;
            m_vrTexture.handle = nullptr;

            // Change UV
            vr::VRTextureBounds_t l_bounds = { 0.f, 1.f, 1.f, 0.f };
            ms_vrOverlay->SetOverlayTextureBounds(m_overlayHandle, &l_bounds);

            if(ms_vrOverlay->CreateOverlay("ovrw.capture.next", "OpenVR Widget - Capture - Next", &m_overlayNextHandle) == vr::VROverlayError_None)
            {
                std::string l_iconPath("\\icons\\next.png");
                l_iconPath.insert(0U, Config::GetDirectory());

                m_nextButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, 0.06f, 0.f));
                m_nextButtonTransform->Update(m_transform);

                ms_vrOverlay->SetOverlayWidthInMeters(m_overlayNextHandle, 0.05f);
                ms_vrOverlay->SetOverlayFromFile(m_overlayNextHandle, l_iconPath.c_str());
                ms_vrOverlay->SetOverlayInputMethod(m_overlayNextHandle, vr::VROverlayInputMethod::VROverlayInputMethod_Mouse);
                ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayNextHandle, vr::TrackingUniverseRawAndUncalibrated, &m_nextButtonTransform->GetMatrixVR());
            }

            if(ms_vrOverlay->CreateOverlay("ovrw.capture.previous", "OpenVR Widget - Capture - Previous", &m_overlayPrevHandle) == vr::VROverlayError_None)
            {
                std::string l_iconPath("\\icons\\prev.png");
                l_iconPath.insert(0U, Config::GetDirectory());

                m_prevButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, 0.f, 0.f));
                m_prevButtonTransform->Update(m_transform);

                ms_vrOverlay->SetOverlayWidthInMeters(m_overlayPrevHandle, 0.05f);
                ms_vrOverlay->SetOverlayFromFile(m_overlayPrevHandle, l_iconPath.c_str());
                ms_vrOverlay->SetOverlayInputMethod(m_overlayPrevHandle, vr::VROverlayInputMethod::VROverlayInputMethod_Mouse);
                ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayPrevHandle, vr::TrackingUniverseRawAndUncalibrated, &m_prevButtonTransform->GetMatrixVR());
            }

            if(ms_vrOverlay->CreateOverlay("ovrw.capture.update", "OpenVR Widget - Capture - Update", &m_overlayUpdateHandle) == vr::VROverlayError_None)
            {
                std::string l_iconPath("\\icons\\upd.png");
                l_iconPath.insert(0U, Config::GetDirectory());

                m_updButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, -0.06f, 0.f));
                m_updButtonTransform->Update(m_transform);

                ms_vrOverlay->SetOverlayWidthInMeters(m_overlayUpdateHandle, 0.05f);
                ms_vrOverlay->SetOverlayFromFile(m_overlayUpdateHandle, l_iconPath.c_str());
                ms_vrOverlay->SetOverlayInputMethod(m_overlayUpdateHandle, vr::VROverlayInputMethod::VROverlayInputMethod_Mouse);
                ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayUpdateHandle, vr::TrackingUniverseRawAndUncalibrated, &m_updButtonTransform->GetMatrixVR());
            }

            m_windowGrabber = new WindowGrabber();
            m_windowGrabber->UpdateWindows();
            m_windowIndex = 0U;

            m_valid = true;
        }
    }
    return m_valid;
}
void WidgetCapture::Destroy()
{
    Cleanup();
}
void WidgetCapture::Update()
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
                        const SL::Screen_Capture::Window *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
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
                        const SL::Screen_Capture::Window *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
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
                        const SL::Screen_Capture::Window *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
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
                            if(m_windowGrabber->StartCapture(m_windowIndex))
                            {
                                m_vrTexture.handle = m_windowGrabber->GetTextureHandle();
                                const SL::Screen_Capture::Window *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
                                if(l_window)
                                {
                                    vr::HmdVector2_t l_scale = { static_cast<float>(l_window->Size.x), static_cast<float>(l_window->Size.y) };
                                    ms_vrOverlay->SetOverlayMouseScale(m_overlayHandle, &l_scale);
                                }
                            }
                            else m_vrTexture.handle = nullptr;
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
                            if(m_windowGrabber->StartCapture(m_windowIndex))
                            {
                                m_vrTexture.handle = m_windowGrabber->GetTextureHandle();
                                const SL::Screen_Capture::Window *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
                                if(l_window)
                                {
                                    vr::HmdVector2_t l_scale = { static_cast<float>(l_window->Size.x), static_cast<float>(l_window->Size.y) };
                                    ms_vrOverlay->SetOverlayMouseScale(m_overlayHandle, &l_scale);
                                    SetForegroundWindow(reinterpret_cast<HWND>(l_window->Handle));
                                }
                            }
                            else m_vrTexture.handle = nullptr;
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
                        if(m_windowGrabber->StartCapture(m_windowIndex))
                        {
                            m_vrTexture.handle = m_windowGrabber->GetTextureHandle();
                            const SL::Screen_Capture::Window *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
                            if(l_window)
                            {
                                vr::HmdVector2_t l_scale = { static_cast<float>(l_window->Size.x), static_cast<float>(l_window->Size.y) };
                                ms_vrOverlay->SetOverlayMouseScale(m_overlayHandle, &l_scale);
                                SetForegroundWindow(reinterpret_cast<HWND>(l_window->Handle));
                            }
                        }
                        else m_vrTexture.handle = nullptr;
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

            m_nextButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, 0.06f, 0.f));
            m_prevButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, 0.f, 0.f));
            m_updButtonTransform->SetPosition(glm::vec3(m_overlayWidth * 0.5f + 0.035f, -0.06f, 0.f));
        }

        m_windowGrabber->Update();
        if(m_vrTexture.handle) ms_vrOverlay->SetOverlayTexture(m_overlayHandle, &m_vrTexture);

        m_transform->Update();
        ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseRawAndUncalibrated, &m_transform->GetMatrixVR());

        m_nextButtonTransform->Update(m_transform);
        m_prevButtonTransform->Update(m_transform);
        m_updButtonTransform->Update(m_transform);
        if(m_activeDashboard)
        {
            ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayNextHandle, vr::TrackingUniverseRawAndUncalibrated, &m_nextButtonTransform->GetMatrixVR());
            ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayPrevHandle, vr::TrackingUniverseRawAndUncalibrated, &m_prevButtonTransform->GetMatrixVR());
            ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayUpdateHandle, vr::TrackingUniverseRawAndUncalibrated, &m_updButtonTransform->GetMatrixVR());
        }
    }
}

void WidgetCapture::OnButtonPress(WidgetHand f_hand, uint32_t f_button)
{
    if(m_valid)
    {
        switch(f_hand)
        {
            case WH_Left:
            {
                if(f_hand == WH_Left)
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
                                    if(m_windowGrabber->StartCapture(m_windowIndex))
                                    {
                                        m_vrTexture.handle = m_windowGrabber->GetTextureHandle();
                                        const SL::Screen_Capture::Window *l_window = m_windowGrabber->GetWindowInfo(m_windowIndex);
                                        if(l_window)
                                        {
                                            vr::HmdVector2_t l_scale = { static_cast<float>(l_window->Size.x), static_cast<float>(l_window->Size.y) };
                                            ms_vrOverlay->SetOverlayMouseScale(m_overlayHandle, &l_scale);
                                            SetForegroundWindow(reinterpret_cast<HWND>(l_window->Handle));
                                        }
                                    }
                                    else m_vrTexture.handle = nullptr;

                                    ms_vrOverlay->ShowOverlay(m_overlayHandle);
                                    if(m_activeDashboard)
                                    {
                                        ms_vrOverlay->ShowOverlay(m_overlayNextHandle);
                                        ms_vrOverlay->ShowOverlay(m_overlayPrevHandle);
                                        ms_vrOverlay->ShowOverlay(m_overlayUpdateHandle);
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
                            if(m_visible && !m_activeDashboard)
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
                }
            } break;

            case WH_Right:
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

void WidgetCapture::OnButtonRelease(WidgetHand f_hand, uint32_t f_button)
{
    if(m_valid && m_visible)
    {
        if((f_hand == WH_Right) && (f_button == vr::k_EButton_SteamVR_Trigger))
        {
            if(m_activeResize) m_activeResize = false;
        }
    }
}

void WidgetCapture::OnDashboardOpen()
{
    if(m_valid)
    {
        m_activeDashboard = true;
        if(m_visible)
        {
            m_activeMove = false;
            m_activeResize = false;

            ms_vrOverlay->ShowOverlay(m_overlayNextHandle);
            ms_vrOverlay->ShowOverlay(m_overlayPrevHandle);
            ms_vrOverlay->ShowOverlay(m_overlayUpdateHandle);
        }
    }
}
void WidgetCapture::OnDashboardClose()
{
    if(m_valid)
    {
        m_activeDashboard = false;
        if(m_visible)
        {
            ms_vrOverlay->HideOverlay(m_overlayNextHandle);
            ms_vrOverlay->HideOverlay(m_overlayPrevHandle);
            ms_vrOverlay->HideOverlay(m_overlayUpdateHandle);
        }
    }
}

//SendMessage(Window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(mouseX, mouseY));
//SendMessage(Window, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(mouseX, mouseY));
