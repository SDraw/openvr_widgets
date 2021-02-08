#pragma once
#include "Widgets/Widget.h"

class GuiSystem;
class GuiElement;
class GuiImage;
class GuiText;
class WindowCapturer;

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
    
    VROverlay *m_overlayBackground;
    VROverlay *m_overlayControls;

    GuiSystem *m_guiSystem;
    GuiImage *m_guiImages[CEI_Count];
    GuiText *m_guiTextWindow;

    static sf::Texture *ms_textureAtlas;

    WindowCapturer *m_windowCapturer;
    size_t m_windowIndex;
#ifdef __linux__
    static Display *ms_display;
#endif

    unsigned long long m_lastLeftTriggerTick;
    unsigned long long m_lastRightTriggerTick;

    bool m_activeMove;
    bool m_activeResize;
    bool m_activePin;

    float m_windowRatio;
    glm::ivec2 m_mousePosition;

    size_t m_fpsMode;

    WidgetWindowCapture(const WidgetWindowCapture &that) = delete;
    WidgetWindowCapture& operator=(const WidgetWindowCapture &that) = delete;

    void StartCapture();

    void OnGuiElementMouseClick(GuiElement *f_guiElement, unsigned char f_button, unsigned char f_state);

    // Widget
    bool Create();
    void Destroy() override;
    void Update();
    void OnButtonPress(size_t f_hand, uint32_t f_button) override;
    void OnButtonRelease(size_t f_hand, uint32_t f_button) override;
    void OnDashboardOpen() override;
    void OnDashboardClose() override;
public:
    WidgetWindowCapture();
    ~WidgetWindowCapture();

    static void InitStaticResources();
    static void RemoveStaticResources();
};
