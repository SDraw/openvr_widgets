#pragma once

class Transformation;
class Widget
{
public:
    enum WidgetHand : unsigned char
    {
        WH_Left = 0U,
        WH_Right
    };

    Widget(const Widget &that) = delete;
    Widget& operator=(const Widget &that) = delete;
protected:
    static vr::IVROverlay *ms_vrOverlay;

    bool m_valid;

    vr::VROverlayHandle_t m_overlayHandle;
    vr::Texture_t m_vrTexture;
    Transformation *m_transform;
    bool m_visible;

    Widget();
    virtual ~Widget();

    virtual void Cleanup() = 0;

    virtual bool Create() = 0;
    virtual void Destroy() = 0;
    virtual void Update() = 0;

    virtual void OnHandActivated(WidgetHand f_hand) {}
    virtual void OnHandDeactivated(WidgetHand f_hand) {}
    virtual void OnButtonPress(WidgetHand f_hand, uint32_t f_button) {}
    virtual void OnButtonRelease(WidgetHand f_hand, uint32_t f_button) {}
    virtual void OnDashboardOpen() {}
    virtual void OnDashboardClose() {}

    static void SetInterfaces(vr::IVROverlay *f_overlay);

    friend class Core;
};

