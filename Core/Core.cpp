#include "stdafx.h"

#include "Core/Core.h"
#include "Core/Config.h"
#include "Core/TransformStorage.h"
#include "Widgets/WidgetWatch.h"
#include "Utils/Utils.h"

Core::Core()
{
    m_vrSystem = nullptr;
    m_vrOverlay = nullptr;
    m_event = { 0 };
    m_context = nullptr;
    m_active = false;
    m_leftHand = vr::k_unTrackedDeviceIndexInvalid;
    m_rightHand = vr::k_unTrackedDeviceIndexInvalid;
}
Core::~Core()
{
    Cleanup();
}

void Core::Cleanup()
{
    for(auto l_widget : m_widgets)
    {
        l_widget->Destroy();
        delete l_widget;
    }
    m_widgets.clear();

    delete m_context;
    m_context = nullptr;

    if(m_vrSystem)
    {
        vr::VR_Shutdown();
        m_vrSystem = nullptr;
    }
}

bool Core::Init()
{
    bool l_result = false;
    if(!m_vrSystem)
    {
        vr::EVRInitError l_initError;
        m_vrSystem = vr::VR_Init(&l_initError, vr::VRApplication_Overlay);
        if(l_initError == vr::VRInitError_None)
        {
            Config::Load();
            m_threadDelay = std::chrono::milliseconds(Config::GetUpdateRate());

            m_vrOverlay = vr::VROverlay();
            m_leftHand = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
            m_rightHand = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);

            sf::ContextSettings l_contextSettings(0U, 0U, 0U, 3U, 1U, sf::ContextSettings::Core, false);
            m_context = new sf::Context(l_contextSettings, 2U, 2U);

            if(m_context->setActive(true))
            {
                m_active = true;

                // All good, create widgets
                Widget::SetInterfaces(m_vrOverlay);

                Widget *l_widget = new WidgetWatch();
                if(l_widget->Create()) m_widgets.push_back(l_widget);
                else
                {
                    delete l_widget;
                    MessageBoxA(NULL, "Unable to create watch widget", NULL, MB_OK | MB_ICONEXCLAMATION);
                }

                // Send hand states to widgets
                if(m_leftHand != vr::k_unTrackedDeviceIndexInvalid)
                {
                    for(auto l_widget : m_widgets) l_widget->OnHandActivated(Widget::WH_Left);
                }
                if(m_rightHand != vr::k_unTrackedDeviceIndexInvalid)
                {
                    for(auto l_widget : m_widgets) l_widget->OnHandActivated(Widget::WH_Right);
                }

                l_result = true;
            }
            else MessageBoxA(NULL, "Unable to create OpenGL 3.1 context", NULL, MB_OK | MB_ICONEXCLAMATION);
        }
        else
        {
            m_vrSystem = nullptr;

            std::string l_errorString("Unable to launch application\nOpenVR description: ");
            l_errorString.append(vr::VR_GetVRInitErrorAsEnglishDescription(l_initError));
            MessageBoxA(NULL, l_errorString.c_str(), NULL, MB_OK | MB_ICONEXCLAMATION);
        }
    }
    return l_result;
}

bool Core::DoPulse()
{
    if(m_active)
    {
        // Save transformations
        vr::TrackedDevicePose_t l_hmdPose;
        m_vrSystem->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseRawAndUncalibrated, 0.f, &l_hmdPose, 1U);
        TransformStorage::SetHmdTransformation(l_hmdPose.mDeviceToAbsoluteTracking);

        if(m_leftHand != vr::k_unTrackedDeviceIndexInvalid)
        {
            vr::TrackedDevicePose_t l_handPose;
            vr::VRControllerState_t l_handState;
            m_vrSystem->GetControllerStateWithPose(vr::TrackingUniverseRawAndUncalibrated, m_leftHand, &l_handState, 1U, &l_handPose);
            TransformStorage::SetLeftHandTransformation(l_handPose.mDeviceToAbsoluteTracking);
        }

        if(m_rightHand != vr::k_unTrackedDeviceIndexInvalid)
        {
            vr::TrackedDevicePose_t l_handPose;
            vr::VRControllerState_t l_handState;
            m_vrSystem->GetControllerStateWithPose(vr::TrackingUniverseRawAndUncalibrated, m_rightHand, &l_handState, 1U, &l_handPose);
            TransformStorage::SetRightHandTransformation(l_handPose.mDeviceToAbsoluteTracking);
        }

        // Poll events
        while(m_vrSystem->PollNextEvent(&m_event, sizeof(vr::VREvent_t)))
        {
            switch(m_event.eventType)
            {
                case vr::VREvent_Quit:
                    m_active = false;
                    break;
                case vr::VREvent_TrackedDeviceActivated:
                {
                    if(m_vrSystem->GetTrackedDeviceClass(m_event.trackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
                    {
                        if(m_leftHand == vr::k_unTrackedDeviceIndexInvalid)
                        {
                            if(m_vrSystem->GetControllerRoleForTrackedDeviceIndex(m_event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand)
                            {
                                m_leftHand = m_event.trackedDeviceIndex;
                                for(auto l_widget : m_widgets) l_widget->OnHandActivated(Widget::WH_Left);
                            }
                        }
                        else
                        {
                            if(m_rightHand == vr::k_unTrackedDeviceIndexInvalid)
                            {
                                if(m_vrSystem->GetControllerRoleForTrackedDeviceIndex(m_event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand)
                                {
                                    m_rightHand = m_event.trackedDeviceIndex;
                                    for(auto l_widget : m_widgets) l_widget->OnHandActivated(Widget::WH_Right);
                                }
                            }
                        }
                    }
                } break;
                case vr::VREvent_TrackedDeviceDeactivated:
                {
                    if(m_leftHand == m_event.trackedDeviceIndex)
                    {
                        m_leftHand = vr::k_unTrackedDeviceIndexInvalid;
                        for(auto l_widget : m_widgets) l_widget->OnHandDeactivated(Widget::WH_Left);
                    }
                    else
                    {
                        if(m_rightHand == m_event.trackedDeviceIndex)
                        {
                            m_rightHand = vr::k_unTrackedDeviceIndexInvalid;
                            for(auto l_widget : m_widgets) l_widget->OnHandDeactivated(Widget::WH_Right);
                        }
                    }
                } break;
                case vr::VREvent_TrackedDeviceRoleChanged:
                {
                    if(m_leftHand == vr::k_unTrackedDeviceIndexInvalid)
                    {
                        m_leftHand = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
                        if(m_leftHand != vr::k_unTrackedDeviceIndexInvalid)
                        {
                            for(auto l_widget : m_widgets) l_widget->OnHandActivated(Widget::WH_Left);
                        }
                    }
                    else
                    {
                        if(m_rightHand == vr::k_unTrackedDeviceIndexInvalid)
                        {
                            m_rightHand = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
                            if(m_rightHand != vr::k_unTrackedDeviceIndexInvalid)
                            {
                                for(auto l_widget : m_widgets) l_widget->OnHandActivated(Widget::WH_Right);
                            }
                        }
                    }
                } break;
                case vr::VREvent_ButtonPress:
                {
                    if((m_event.trackedDeviceIndex == m_leftHand) || (m_event.trackedDeviceIndex == m_rightHand))
                    {
                        for(auto l_widget : m_widgets) l_widget->OnButtonPress((m_event.trackedDeviceIndex == m_leftHand) ? Widget::WH_Left : Widget::WH_Right, m_event.data.controller.button);
                    }
                } break;
                case vr::VREvent_ButtonUnpress:
                {
                    if((m_event.trackedDeviceIndex == m_leftHand) || (m_event.trackedDeviceIndex == m_rightHand))
                    {
                        for(auto l_widget : m_widgets) l_widget->OnButtonRelease((m_event.trackedDeviceIndex == m_leftHand) ? Widget::WH_Left : Widget::WH_Right, m_event.data.controller.button);
                    }
                } break;
            }
        }

        if(m_active)
        {
            for(auto l_widget : m_widgets) l_widget->Update();

            std::this_thread::sleep_for(m_threadDelay);
        }
    }
    return m_active;
}

void Core::Terminate()
{
    Cleanup();
}
