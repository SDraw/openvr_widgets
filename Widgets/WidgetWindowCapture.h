#pragma once
#include "Widgets/Widget.h"

class Transformation;
class WindowGrabber;

class WidgetWindowCapture final : public Widget
{
    static sf::Texture *ms_iconsAtlas;
    static vr::Texture_t ms_textureControls;

    enum ControlIndex : size_t
    {
        ControlIndex_PinUnpin = 0U,
        ControlIndex_Close,
        ControlIndex_Previous,
        ControlIndex_Next,
        ControlIndex_Update,
        ControlIndex_ChangeFps,

        ControlIndex_Max
    };
    enum FpsMode : size_t
    {
        FpsMode_15 = 0U,
        FpsMode_30,
        FpsMode_60,

        FpsMode_Max
    };

    vr::VROverlayHandle_t m_overlayControlHandles[ControlIndex_Max];
    vr::VREvent_t m_overlayEvent;

    WindowGrabber *m_windowGrabber;
    size_t m_windowIndex;

    ULONGLONG m_lastLeftTriggerTick;
    ULONGLONG m_lastRightTriggerTick;
    bool m_closed;
    bool m_activeDashboard;
    bool m_activeMove;
    bool m_activeResize;
    bool m_activePin;
    size_t m_fpsMode;
    float m_overlayWidth;
    glm::ivec2 m_windowSize;
    glm::ivec2 m_mousePosition;
    Transformation *m_transformControls[ControlIndex_Max];


    WidgetWindowCapture(const WidgetWindowCapture &that) = delete;
    WidgetWindowCapture& operator=(const WidgetWindowCapture &that) = delete;

    void InternalStartCapture();

    // Widget
    bool Create();
    void Destroy();
    void Cleanup();
    void Update();
    bool CloseRequested() const;
    void OnButtonPress(unsigned char f_hand, uint32_t f_button);
    void OnButtonRelease(unsigned char  f_hand, uint32_t f_button);
    void OnDashboardOpen();
    void OnDashboardClose();
protected:
    WidgetWindowCapture();
    ~WidgetWindowCapture();

    static void RemoveIconsAtlas();

    friend class WidgetManager;
};

