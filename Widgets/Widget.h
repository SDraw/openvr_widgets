#pragma once

class Transformation;
class Widget
{
    Widget(const Widget &that) = delete;
    Widget& operator=(const Widget &that) = delete;
protected:
    vr::VROverlayHandle_t m_overlay;
    vr::Texture_t m_texture;
    vr::VREvent_t m_event;

    bool m_valid;
    bool m_visible; // Used for constant widgets in general
    bool m_closed;
    bool m_activeDashboard;
    Transformation *m_transform;

    Widget();
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

    friend class WidgetManager;
};
