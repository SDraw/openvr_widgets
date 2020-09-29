#pragma once

class ConfigManager;
class WidgetManager;

class Core final
{
    vr::IVRSystem *m_vrSystem;
    vr::VREvent_t m_event;
    std::vector<vr::TrackedDeviceIndex_t> m_deviceIndex;

    bool m_active;
    sf::Context *m_context;
    std::chrono::milliseconds m_threadDelay;
    unsigned long long m_systemTick;
    unsigned long long m_powerTick;

    ConfigManager *m_configManager;
    WidgetManager *m_widgetManager;

    Core(const Core &that) = delete;
    Core& operator=(const Core &that) = delete;
public:
    Core();
    ~Core();

    bool Initialize();
    void Terminate();
    bool DoPulse();

    inline vr::IVRSystem* GetVRSystem() const { return m_vrSystem; }
    inline WidgetManager* GetWidgetManager() const { return m_widgetManager; }

    void ForceHandSearch();
    void SendMessageToDeviceWithProperty(uint64_t f_value, const char *f_message);
};
