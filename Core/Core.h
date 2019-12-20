#pragma once

class ConfigManager;
class WidgetManager;

class Core final
{
    vr::IVRSystem *m_vrSystem;
    vr::IVROverlay *m_vrOverlay;
    vr::IVRCompositor *m_vrCompositor;
    vr::IVRChaperone *m_vrChaperone;
    vr::VREvent_t m_event;

    sf::Context *m_context;
    std::chrono::milliseconds m_threadDelay;
    bool m_active;

    vr::TrackedDeviceIndex_t m_leftHand;
    vr::TrackedDeviceIndex_t m_rightHand;
    
    ConfigManager *m_configManager;
    WidgetManager *m_widgetManager;

    Core(const Core &that) = delete;
    Core& operator=(const Core &that) = delete;

    void Cleanup();
public:
    Core();
    ~Core();

    bool Init();
    bool DoPulse();
    void Terminate();

    inline vr::IVRSystem* GetVRSystem() const { return m_vrSystem; }
    inline vr::IVROverlay* GetVROverlay() const { return m_vrOverlay; }
    inline vr::IVRCompositor* GetVRCompositor() const { return m_vrCompositor; }
    inline vr::IVRChaperone* GetVRChaperone() const { return m_vrChaperone; }

    inline WidgetManager* GetWidgetManager() const { return m_widgetManager; }
    inline ConfigManager* GetConfigManager() const { return m_configManager; }
};

