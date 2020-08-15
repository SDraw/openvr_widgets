#pragma once

class ConfigManager;
class WidgetManager;

class Core final
{
    vr::IVRSystem *m_vrSystem;
    vr::IVROverlay *m_vrOverlay;
    vr::IVRCompositor *m_vrCompositor;
    vr::IVRDebug *m_vrDebug;
    vr::VREvent_t m_event;
    vr::TrackedDeviceIndex_t m_leftHand;
    vr::TrackedDeviceIndex_t m_rightHand;

    bool m_active;
    sf::Context *m_context;
    std::chrono::milliseconds m_threadDelay;
    unsigned long long m_powerTick;

    ConfigManager *m_configManager;
    WidgetManager *m_widgetManager;

    Core(const Core &that) = delete;
    Core& operator=(const Core &that) = delete;

    void Cleanup();
public:
    Core();
    ~Core();

    bool Initialize();
    void Terminate();
    bool DoPulse();

    inline vr::IVRSystem* GetVRSystem() const { return m_vrSystem; }
    inline vr::IVROverlay* GetVROverlay() const { return m_vrOverlay; }
    inline vr::IVRCompositor* GetVRCompositor() const { return m_vrCompositor; }

    inline WidgetManager* GetWidgetManager() const { return m_widgetManager; }
    inline ConfigManager* GetConfigManager() const { return m_configManager; }

    void ForceHandSearch();
    void SendMessageToDeviceWithProperty(uint64_t f_value, const char *f_message);
};
