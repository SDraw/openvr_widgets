#include "stdafx.h"

#include "Core/Core.h"
#include "Managers/ConfigManager.h"
#include "Managers/WidgetManager.h"

#include "Core/VRDevicesStates.h"
#include "Utils/Utils.h"

SFML_DEFINE_DISCRETE_GPU_PREFERENCE

const unsigned long long g_powerUpdateInterval = 30000U;

Core::Core()
{
    m_vrSystem = nullptr;
    m_event = { 0 };
    m_deviceIndex.assign(VRDeviceIndex::VDI_Max, vr::k_unTrackedDeviceIndexInvalid);

    m_context = nullptr;
    m_active = false;
    m_systemTick = GetTickCount64();
    m_powerTick = 0U;

    m_configManager = nullptr;
    m_widgetManager = nullptr;
}
Core::~Core()
{
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
                // Load settings
                m_configManager = new ConfigManager();
                m_configManager->Load();
                m_threadDelay = std::chrono::milliseconds(ConfigManager::GetUpdateDelay());

                // Init context
                const sf::ContextSettings l_contextSettings(0U, 0U, 0U, 3U, 0U, sf::ContextSettings::Core, false);
                m_context = new sf::Context(l_contextSettings, 2U, 2U);
                if(m_context->setActive(true))
                {
                    // Find devices
                    m_deviceIndex[VRDeviceIndex::VDI_Hmd] = 0U; // Always has been
                    m_deviceIndex[VRDeviceIndex::VDI_LeftController] = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
                    m_deviceIndex[VRDeviceIndex::VDI_RightController] = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);

                    for(vr::TrackedDeviceIndex_t i = 1U; i < vr::k_unMaxTrackedDeviceCount; i++)
                    {
                        if(m_vrSystem->IsTrackedDeviceConnected(i))
                        {
                            if(m_vrSystem->GetTrackedDeviceClass(i) == vr::TrackedDeviceClass_GenericTracker)
                            {
                                for(size_t j = VRDeviceIndex::VDI_FirstTracker; j <= VRDeviceIndex::VDI_ThirdTracker; j++)
                                {
                                    if(m_deviceIndex[j] == vr::k_unTrackedDeviceIndexInvalid)
                                    {
                                        m_deviceIndex[j] = i;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // Init widgets
                    m_widgetManager = new WidgetManager(this);
                    if(m_deviceIndex[VRDeviceIndex::VDI_LeftController] != vr::k_unTrackedDeviceIndexInvalid) m_widgetManager->OnHandActivated(VRDeviceIndex::VDI_LeftController);
                    if(m_deviceIndex[VRDeviceIndex::VDI_RightController] != vr::k_unTrackedDeviceIndexInvalid) m_widgetManager->OnHandActivated(VRDeviceIndex::VDI_RightController);
                    if(vr::VROverlay()->IsDashboardVisible()) m_widgetManager->OnDashboardOpen();

                    m_active = true;
                }
                else
                {
#ifdef _WIN32
                    MessageBoxA(NULL, "Unable to create OpenGL 3.0 (or higher) context", NULL, MB_OK | MB_ICONEXCLAMATION);
#elif __linux__
                    std::cerr << "Unable to create OpenGL 3.0 (or higher) context" << std::endl;
#endif
                }
            }
            else
            {
                m_vrSystem = nullptr;

                std::string l_errorString("Unable to launch application\nOpenVR description: ");
                l_errorString.append(vr::VR_GetVRInitErrorAsEnglishDescription(l_initError));
#ifdef _WIN32
                MessageBoxA(NULL, l_errorString.c_str(), NULL, MB_OK | MB_ICONEXCLAMATION);
#elif __linux__
                std::cerr << l_errorString.c_str() << std::endl;
#endif
            }
        }
    }
    return m_active;
}

void Core::Terminate()
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
    }
}

bool Core::DoPulse()
{
    if(m_active)
    {
        // Update tick
        m_systemTick = GetTickCount64();

        // Save transformations
        vr::TrackedDevicePose_t l_pose[vr::k_unMaxTrackedDeviceCount];
        m_vrSystem->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseRawAndUncalibrated, 0.f, l_pose, vr::k_unMaxTrackedDeviceCount);

        for(size_t i = 0U; i < VRDeviceIndex::VDI_Max; i++)
        {
            if(m_deviceIndex[i] != vr::k_unTrackedDeviceIndexInvalid)
            {
                VRDevicesStates::SetDeviceTransformation(i, l_pose[m_deviceIndex[i]].mDeviceToAbsoluteTracking);
            }
        }

        // Get battery levels
        if((m_systemTick - m_powerTick) >= g_powerUpdateInterval) // Check every 30 seconds
        {
            m_powerTick = m_systemTick;
            for(size_t i = 0U; i < VRDeviceIndex::VDI_Max; i++)
            {
                if(m_deviceIndex[i] != vr::k_unTrackedDeviceIndexInvalid)
                {
                    VRDevicesStates::SetDevicePower(i, m_vrSystem->GetFloatTrackedDeviceProperty(m_deviceIndex[i], vr::ETrackedDeviceProperty::Prop_DeviceBatteryPercentage_Float)*100.f);
                }
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
                    switch(m_vrSystem->GetTrackedDeviceClass(m_event.trackedDeviceIndex))
                    {
                        case vr::TrackedDeviceClass_Controller:
                        {
                            VRDeviceIndex l_deviceIndex = VRDeviceIndex::VDI_Max;
                            switch(m_vrSystem->GetControllerRoleForTrackedDeviceIndex(m_event.trackedDeviceIndex))
                            {
                                case vr::TrackedControllerRole_LeftHand:
                                    l_deviceIndex = VRDeviceIndex::VDI_LeftController;
                                    break;
                                case vr::TrackedControllerRole_RightHand:
                                    l_deviceIndex = VRDeviceIndex::VDI_RightController;
                                    break;
                            }
                            if(l_deviceIndex != VRDeviceIndex::VDI_Max)
                            {
                                if(m_deviceIndex[l_deviceIndex] == vr::k_unTrackedDeviceIndexInvalid)
                                {
                                    m_deviceIndex[l_deviceIndex] = m_event.trackedDeviceIndex;
                                    m_widgetManager->OnHandActivated(l_deviceIndex);
                                }
                            }
                        } break;

                        case vr::TrackedDeviceClass_GenericTracker:
                        {
                            for(size_t i = VRDeviceIndex::VDI_FirstTracker; i <= VRDeviceIndex::VDI_ThirdTracker; i++)
                            {
                                if(m_deviceIndex[i] == vr::k_unTrackedDeviceIndexInvalid)
                                {
                                    m_deviceIndex[i] = m_event.trackedDeviceIndex;
                                    m_powerTick -= g_powerUpdateInterval;
                                    break;
                                }
                            }
                        } break;
                    }
                } break;

                case vr::VREvent_TrackedDeviceDeactivated:
                {
                    for(size_t i = 0U; i < VRDeviceIndex::VDI_Max; i++)
                    {
                        if(m_deviceIndex[i] == m_event.trackedDeviceIndex)
                        {
                            switch(i)
                            {
                                case VRDeviceIndex::VDI_LeftController: case VRDeviceIndex::VDI_RightController:
                                    m_widgetManager->OnHandDeactivated(i);
                                    break;
                            }

                            VRDevicesStates::SetDevicePower(i, 0.f);
                            m_deviceIndex[i] = vr::k_unTrackedDeviceIndexInvalid;
                            break;
                        }
                    }
                } break;

                case vr::VREvent_TrackedDeviceRoleChanged:
                {
                    // m_event.trackedDeviceIndex is invalid in this event, Valve plz fix
                    for(size_t i = VDI_LeftController; i <= VDI_RightController; i++)
                    {
                        if(m_deviceIndex[i] == vr::k_unTrackedDeviceIndexInvalid)
                        {
                            vr::TrackedDeviceIndex_t l_controllerIndex = m_vrSystem->GetTrackedDeviceIndexForControllerRole((i == VRDeviceIndex::VDI_LeftController) ? vr::TrackedControllerRole_LeftHand : vr::TrackedControllerRole_RightHand);
                            if(l_controllerIndex != vr::k_unTrackedDeviceIndexInvalid)
                            {
                                m_deviceIndex[i] = l_controllerIndex;
                                m_widgetManager->OnHandActivated(i);
                                m_powerTick -= g_powerUpdateInterval;
                                break;
                            }
                        }
                    }
                } break;

                case vr::VREvent_ButtonPress:
                {
                    for(size_t i = VDI_LeftController; i <= VDI_RightController; i++)
                    {
                        if(m_deviceIndex[i] == m_event.trackedDeviceIndex)
                        {
                            m_widgetManager->OnButtonPress(i, m_event.data.controller.button);
                            break;
                        }
                    }
                } break;

                case vr::VREvent_ButtonUnpress:
                {
                    for(size_t i = VDI_LeftController; i <= VDI_RightController; i++)
                    {
                        if(m_deviceIndex[i] == m_event.trackedDeviceIndex)
                        {
                            m_widgetManager->OnButtonRelease(i, m_event.data.controller.button);
                            break;
                        }
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
        m_deviceIndex[VRDeviceIndex::VDI_LeftController] = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
        m_deviceIndex[VRDeviceIndex::VDI_RightController] = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
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
                    vr::VRDebug()->DriverDebugRequest(i, f_message, l_response, 32U);
                    break;
                }
            }
        }
    }
}
