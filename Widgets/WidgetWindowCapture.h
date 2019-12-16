#pragma once
#include "Widgets/Widget.h"

class Transformation;
class WindowGrabber;

class WidgetWindowCapture final : public Widget
{
    vr::VROverlayHandle_t m_overlayNextHandle;
    vr::VROverlayHandle_t m_overlayPrevHandle;
    vr::VROverlayHandle_t m_overlayUpdateHandle;
    vr::VROverlayHandle_t m_overlayPinHandle;
    vr::VREvent_t m_overlayEvent;

    WindowGrabber *m_windowGrabber;
    size_t m_windowIndex;

    ULONGLONG m_lastLeftGripTick;
    ULONGLONG m_lastLeftTriggerTick;
    ULONGLONG m_lastRightTriggerTick;
    bool m_activeDashboard;
    bool m_activeMove;
    bool m_activeResize;
    bool m_activePin;
    float m_overlayWidth;
    glm::ivec2 m_windowSize;
    glm::ivec2 m_mousePosition;
    Transformation *m_pinButtonTransform;
    Transformation *m_nextButtonTransform;
    Transformation *m_prevButtonTransform;
    Transformation *m_updButtonTransform;

    WidgetWindowCapture(const WidgetWindowCapture &that) = delete;
    WidgetWindowCapture& operator=(const WidgetWindowCapture &that) = delete;

    void Cleanup();
    void InternalStartCapture();

    // Widget
    bool Create();
    void Destroy();
    void Update();
    void OnButtonPress(unsigned char f_hand, uint32_t f_button);
    void OnButtonRelease(unsigned char  f_hand, uint32_t f_button);
    void OnDashboardOpen();
    void OnDashboardClose();
protected:
    WidgetWindowCapture();
    ~WidgetWindowCapture();

    friend class WidgetManager;
};

