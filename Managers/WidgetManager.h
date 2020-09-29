#pragma once

class Core;
class GuiSystem;
class GuiElement;
class GuiButton;
class Widget;

class WidgetManager final
{
    enum GuiElementIndex : size_t
    {
        GEI_AddWindowCapture = 0U,
        GEI_AddKeyboard,
        GEI_ReassignHands,
        GEI_SwitchKinectTracking,
        GEI_SwitchLeapLeftHand,
        GEI_SwitchLeapRightHand,

        GEI_Max
    };

    Core *m_core;

    vr::VROverlayHandle_t m_overlayDashboardThumbnail;
    vr::VROverlayHandle_t m_overlayDashboard;
    vr::Texture_t m_textureDashboard;
    vr::VREvent_t m_overlayEvent;

    std::map<unsigned char, Widget*> m_constantWidgets;
    std::vector<Widget*> m_widgets;

    GuiSystem *m_guiSystem;
    GuiButton *m_guiElements[GEI_Max];
    bool m_activeDashboard;

    WidgetManager(const WidgetManager &that) = delete;
    WidgetManager& operator=(const WidgetManager &that) = delete;

    void OnGuiElementMouseClick(GuiElement *f_guiElement, unsigned char f_button, unsigned char f_state);
public:
    explicit WidgetManager(Core *f_core);
    ~WidgetManager();

    void DoPulse();

    // Events for widgets
    void OnHandActivated(size_t f_hand);
    void OnHandDeactivated(size_t f_hand);
    void OnButtonPress(size_t f_hand, uint32_t f_button);
    void OnButtonRelease(size_t f_hand, uint32_t f_button);
    void OnDashboardOpen();
    void OnDashboardClose();
};
