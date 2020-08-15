#pragma once
#include "Widgets/Widget.h"

class GuiSystem;
class GuiElement;
class GuiImage;
class GuiText;
class Transformation;
class WindowGrabber;

class WidgetWindowCapture final : public Widget
{
    enum GuiElementIndex : size_t
    {
        CEI_Pin = 0U,
        CEI_Close,
        CEI_Previous,
        CEI_Next,
        CEI_Update,
        CEI_FPS,

        CEI_Count
    };
    enum FpsMode : size_t
    {
        FM_15 = 0U,
        FM_30,
        FM_60,

        FM_Count
    };

    vr::VROverlayHandle_t m_overlayControl;

    GuiSystem *m_guiSystem;
    GuiImage *m_guiImages[CEI_Count];
    GuiText *m_guiTextWindow;

    static sf::Texture *ms_textureAtlas;
    vr::Texture_t m_textureControls;

    WindowGrabber *m_windowGrabber;
    size_t m_windowIndex;

    unsigned long long m_lastLeftTriggerTick;
    unsigned long long m_lastRightTriggerTick;

    bool m_activeMove;
    bool m_activeResize;
    bool m_activePin;

    float m_overlayWidth;
    glm::ivec2 m_windowSize;
    glm::ivec2 m_mousePosition;
    Transformation *m_transformControl;

    size_t m_fpsMode;

    WidgetWindowCapture(const WidgetWindowCapture &that) = delete;
    WidgetWindowCapture& operator=(const WidgetWindowCapture &that) = delete;

    void StartCapture();

    // Widget
    bool Create();
    void Destroy() override;
    void Update();
    void OnButtonPress(unsigned char f_hand, uint32_t f_button) override;
    void OnButtonRelease(unsigned char  f_hand, uint32_t f_button) override;
    void OnDashboardOpen() override;
    void OnDashboardClose() override;
public:
    void OnGuiElementMouseClick(GuiElement *f_guiElement, unsigned char f_button, unsigned char f_state);
protected:
    WidgetWindowCapture();
    ~WidgetWindowCapture();

    static void RemoveStaticResources();

    friend class WidgetManager;
};
