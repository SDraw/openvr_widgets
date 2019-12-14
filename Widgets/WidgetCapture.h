#pragma once
#include "Widgets/Widget.h"

class Transformation;
class WindowGrabber;
class WidgetCapture final : public Widget
{
    vr::VROverlayHandle_t m_overlayNextHandle;
    vr::VROverlayHandle_t m_overlayPrevHandle;
    vr::VROverlayHandle_t m_overlayUpdateHandle;
    vr::VREvent_t m_overlayEvent;

    WindowGrabber *m_windowGrabber;
    size_t m_windowIndex;

    ULONGLONG m_lastLeftGripTick;
    ULONGLONG m_lastLeftTriggerTick;
    ULONGLONG m_lastRightTriggerTick;
    bool m_activeDashboard;
    bool m_activeMove;
    bool m_activeResize;
    float m_overlayWidth;
    glm::ivec2 m_windowSize;
    glm::ivec2 m_mousePosition;
    Transformation *m_nextButtonTransform;
    Transformation *m_prevButtonTransform;
    Transformation *m_updButtonTransform;

    void Cleanup();

    WidgetCapture(const WidgetCapture &that) = delete;
    WidgetCapture& operator=(const WidgetCapture &that) = delete;
public:
    WidgetCapture();
    ~WidgetCapture();

    bool Create();
    void Destroy();
    void Update();

protected:
    void OnButtonPress(WidgetHand f_hand, uint32_t f_button);
    void OnButtonRelease(WidgetHand f_hand, uint32_t f_button);
    void OnDashboardOpen();
    void OnDashboardClose();
};

