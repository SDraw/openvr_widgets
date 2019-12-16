#pragma once

class Transformation;
class Widget
{
    Widget(const Widget &that) = delete;
    Widget& operator=(const Widget &that) = delete;
protected:
    Widget();
    virtual ~Widget();

    static vr::IVROverlay *ms_vrOverlay;

    bool m_valid;
    vr::VROverlayHandle_t m_overlayHandle;
    vr::Texture_t m_vrTexture;
    Transformation *m_transform;
    bool m_visible;
    unsigned char m_language;

    virtual void Cleanup() = 0;

    virtual bool Create() = 0;
    virtual void Destroy() = 0;
    virtual void Update() = 0;

    virtual void OnHandActivated(unsigned char f_hand) {}
    virtual void OnHandDeactivated(unsigned char f_hand) {}
    virtual void OnButtonPress(unsigned char f_hand, uint32_t f_button) {}
    virtual void OnButtonRelease(unsigned char f_hand, uint32_t f_button) {}
    virtual void OnDashboardOpen() {}
    virtual void OnDashboardClose() {}
    virtual void OnLanguageChange(unsigned char f_lang);

    static void SetInterfaces(vr::IVROverlay *f_overlay);

    friend class WidgetManager;
};

