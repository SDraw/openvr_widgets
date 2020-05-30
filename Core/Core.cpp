#include "stdafx.h"

#include "Core/Core.h"
#include "Managers/ConfigManager.h"
#include "Managers/WidgetManager.h"

#include "Core/VRTransform.h"
#include "Utils/GlobalStructures.h"
#include "Utils/Utils.h"

SFML_DEFINE_DISCRETE_GPU_PREFERENCE

const ULONGLONG g_PowerUpdateInterval = 30000U;

Core::Core()
{
    m_vrSystem = nullptr;
    m_vrOverlay = nullptr;
    m_vrCompositor = nullptr;
    m_vrDebug = nullptr;
    m_event = { 0 };
    m_leftHand = vr::k_unTrackedDeviceIndexInvalid;
    m_rightHand = vr::k_unTrackedDeviceIndexInvalid;

    m_context = nullptr;
    m_active = false;
    m_powerTick = 0U;

    m_configManager = nullptr;
    m_widgetManager = nullptr;
}
Core::~Core()
{
    Cleanup();
}

void Core::Cleanup()
{
    delete m_widgetManager;
    m_widgetManager = nullptr;

    delete m_configManager;
    m_configManager = nullptr;

    delete m_context;
    m_context = nullptr;

    if(m_vrSystem)
    {
        vr::VR_Shutdown();
        m_vrSystem = nullptr;
        m_vrOverlay = nullptr;
        m_vrCompositor = nullptr;
        m_vrDebug = nullptr;
    }
}

bool Core::Initialize()
{
    if(!m_active)
    {
        if(!m_vrSystem)
        {
            vr::EVRInitError l_initError;
            m_vrSystem = vr::VR_Init(&l_initError, vr::VRApplication_Overlay);

            if(l_initError == vr::VRInitError_None)
            {
                m_vrOverlay = vr::VROverlay();
                m_vrCompositor = vr::VRCompositor();
                m_vrDebug = vr::VRDebug();

                m_leftHand = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
                m_rightHand = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);

                const sf::ContextSettings l_contextSettings(0U, 0U, 0U, 3U, 0U, sf::ContextSettings::Core, false);
                m_context = new sf::Context(l_contextSettings, 2U, 2U);

                if(m_context->setActive(true))
                {
                    m_configManager = new ConfigManager(this);
                    m_configManager->Load();
                    m_threadDelay = std::chrono::milliseconds(m_configManager->GetUpdateDelay());

                    m_widgetManager = new WidgetManager(this);
                    if(m_leftHand != vr::k_unTrackedDeviceIndexInvalid) m_widgetManager->OnHandActivated(VRHandIndex::VRHI_Left);
                    if(m_rightHand != vr::k_unTrackedDeviceIndexInvalid) m_widgetManager->OnHandActivated(VRHandIndex::VRHI_Right);
                    if(m_vrOverlay->IsDashboardVisible()) m_widgetManager->OnDashboardOpen();

                    m_active = true;
                }
                else MessageBoxA(NULL, "Unable to create OpenGL 3.0 (or higher) context", NULL, MB_OK | MB_ICONEXCLAMATION);
            }
            else
            {
                m_vrSystem = nullptr;

                std::string l_errorString("Unable to launch application\nOpenVR description: ");
                l_errorString.append(vr::VR_GetVRInitErrorAsEnglishDescription(l_initError));
                MessageBoxA(NULL, l_errorString.c_str(), NULL, MB_OK | MB_ICONEXCLAMATION);
            }
        }
    }
    return m_active;
}

void Core::Terminate()
{
    m_configManager->Save();
    Cleanup();
}

bool Core::DoPulse()
{
    if(m_active)
    {
        // Save transformations
        vr::TrackedDevicePose_t l_pose[vr::k_unMaxTrackedDeviceCount];
        m_vrSystem->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseRawAndUncalibrated, 0.f, l_pose, vr::k_unMaxTrackedDeviceCount);
        VRTransform::SetHmdTransformation(l_pose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking);

        if(m_leftHand != vr::k_unTrackedDeviceIndexInvalid)
        {
            VRTransform::SetLeftHandTransformation(l_pose[m_leftHand].mDeviceToAbsoluteTracking);
        }
        if(m_rightHand != vr::k_unTrackedDeviceIndexInvalid)
        {
            VRTransform::SetRightHandTransformation(l_pose[m_rightHand].mDeviceToAbsoluteTracking);
        }

        // Get battery levels
        ULONGLONG l_tick = GetTickCount64();
        if((l_tick - m_powerTick) >= g_PowerUpdateInterval) // Check every 30 seconds
        {
            m_powerTick = l_tick;
            if(m_leftHand != vr::k_unTrackedDeviceIndexInvalid)
            {
                VRTransform::SetLeftHandPower(m_vrSystem->GetFloatTrackedDeviceProperty(m_leftHand, vr::ETrackedDeviceProperty::Prop_DeviceBatteryPercentage_Float)*100.f);
            }

            if(m_rightHand != vr::k_unTrackedDeviceIndexInvalid)
            {
                VRTransform::SetRightHandPower(m_vrSystem->GetFloatTrackedDeviceProperty(m_rightHand, vr::ETrackedDeviceProperty::Prop_DeviceBatteryPercentage_Float)*100.f);
            }
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
                                m_widgetManager->OnHandActivated(VRHandIndex::VRHI_Left);
                                m_powerTick -= g_PowerUpdateInterval;
                            }
                        }
                        else
                        {
                            if(m_rightHand == vr::k_unTrackedDeviceIndexInvalid)
                            {
                                if(m_vrSystem->GetControllerRoleForTrackedDeviceIndex(m_event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand)
                                {
                                    m_rightHand = m_event.trackedDeviceIndex;
                                    m_widgetManager->OnHandActivated(VRHandIndex::VRHI_Right);
                                    m_powerTick -= g_PowerUpdateInterval;
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
                        m_widgetManager->OnHandDeactivated(VRHandIndex::VRHI_Left);
                        VRTransform::SetLeftHandPower(0.f);
                    }
                    else
                    {
                        if(m_rightHand == m_event.trackedDeviceIndex)
                        {
                            m_rightHand = vr::k_unTrackedDeviceIndexInvalid;
                            m_widgetManager->OnHandDeactivated(VRHandIndex::VRHI_Right);
                            VRTransform::SetRightHandPower(0.f);
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
                            m_widgetManager->OnHandActivated(VRHandIndex::VRHI_Left);
                            m_powerTick -= g_PowerUpdateInterval;
                        }
                    }
                    else
                    {
                        if(m_rightHand == vr::k_unTrackedDeviceIndexInvalid)
                        {
                            m_rightHand = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
                            if(m_rightHand != vr::k_unTrackedDeviceIndexInvalid)
                            {
                                m_widgetManager->OnHandActivated(VRHandIndex::VRHI_Right);
                                m_powerTick -= g_PowerUpdateInterval;
                            }
                        }
                    }
                } break;
                case vr::VREvent_ButtonPress:
                {
                    if((m_event.trackedDeviceIndex == m_leftHand) || (m_event.trackedDeviceIndex == m_rightHand))
                    {
                        m_widgetManager->OnButtonPress((m_event.trackedDeviceIndex == m_leftHand) ? VRHandIndex::VRHI_Left : VRHandIndex::VRHI_Right, m_event.data.controller.button);
                    }
                } break;
                case vr::VREvent_ButtonUnpress:
                {
                    if((m_event.trackedDeviceIndex == m_leftHand) || (m_event.trackedDeviceIndex == m_rightHand))
                    {
                        m_widgetManager->OnButtonRelease((m_event.trackedDeviceIndex == m_leftHand) ? VRHandIndex::VRHI_Left : VRHandIndex::VRHI_Right, m_event.data.controller.button);
                    }
                } break;
                case vr::VREvent_DashboardActivated:
                    m_widgetManager->OnDashboardOpen();
                    break;
                case vr::VREvent_DashboardDeactivated:
                    m_widgetManager->OnDashboardClose();
                    break;
            }
        }

        if(m_active)
        {
            m_widgetManager->DoPulse();
            std::this_thread::sleep_for(m_threadDelay);
        }
    }
    return m_active;
}

void Core::ForceHandSearch()
{
    if(m_active)
    {
        m_leftHand = vr::k_unTrackedDeviceIndexInvalid;
        m_rightHand = vr::k_unTrackedDeviceIndexInvalid;

        m_leftHand = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
        m_rightHand = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
    }
}

void Core::SendMessageToDeviceWithProperty(uint64_t f_value, const char *f_message)
{
    if(m_active)
    {
        for(uint32_t i = 0U; i < vr::k_unMaxTrackedDeviceCount; i++)
        {
            if(m_vrSystem->IsTrackedDeviceConnected(i))
            {
                if(m_vrSystem->GetUint64TrackedDeviceProperty(i, vr::Prop_VendorSpecific_Reserved_Start) == f_value)
                {
                    char l_response[32U];
                    m_vrDebug->DriverDebugRequest(i, f_message, l_response, 32U);
                    break;
                }
            }
        }
    }
}
