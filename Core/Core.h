#pragma once

class Widget;

class Core final
{
    vr::IVRSystem *m_vrSystem;
    vr::IVROverlay *m_vrOverlay;
    vr::VREvent_t m_event;

    sf::Context *m_context;
    std::chrono::milliseconds m_threadDelay;
    bool m_active;

    vr::TrackedDeviceIndex_t m_leftHand;
    vr::TrackedDeviceIndex_t m_rightHand;
    std::vector<Widget*> m_widgets;

    Core(const Core &that) = delete;
    Core& operator=(const Core &that) = delete;

    void Cleanup();
public:
    Core();
    ~Core();

    bool Init();
    bool DoPulse();
    void Terminate();
};

