#pragma once

class VROverlay;

class Widget
{
    Widget(const Widget &that) = delete;
    Widget& operator=(const Widget &that) = delete;
public:
    virtual ~Widget();

    virtual bool Create() = 0;
    virtual void Destroy();
    virtual void Update() = 0;

    bool IsClosed() const;

    virtual void OnHandActivated(size_t f_hand);
    virtual void OnHandDeactivated(size_t f_hand);
    virtual void OnButtonPress(size_t f_hand, uint32_t f_button);
    virtual void OnButtonRelease(size_t f_hand, uint32_t f_button);
    virtual void OnDashboardOpen();
    virtual void OnDashboardClose();
protected:
    VROverlay *m_overlayMain;
    vr::VREvent_t m_event;
    glm::vec2 m_size;

    bool m_valid;
    bool m_visible; // Used for constant widgets in general
    bool m_closed;
    bool m_activeDashboard;

    Widget();
};
