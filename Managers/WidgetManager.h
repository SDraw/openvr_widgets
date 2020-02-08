#pragma once

class Core;
class GuiSystem;
class GuiElement;
class Widget;

class WidgetManager final
{
    enum GuiElementIndex : size_t
    {
        GuiElementIndex_AddCaptureWindow = 0U,
        GuiElementIndex_ReassignHands,
        GuiElementIndex_SwitchKinectTracking,
        GuiElementIndex_SwitchLeapLeftHand,
        GuiElementIndex_SwitchLeapRightHand,

        GuiElementIndex_Max
    };

    Core *m_core;

    vr::VROverlayHandle_t m_overlayDashboardThumbnail;
    vr::VROverlayHandle_t m_overlayDashboard;
    vr::Texture_t m_textureDashboard;
    vr::VREvent_t m_overlayEvent;

    std::map<unsigned char, Widget*> m_constantWidgets;
    std::vector<Widget*> m_widgets;

    GuiSystem *m_guiSystem;
    GuiElement *m_guiElements[GuiElementIndex_Max];
    bool m_activeDashboard;

    WidgetManager(const WidgetManager &that) = delete;
    WidgetManager& operator=(const WidgetManager &that) = delete;
public:
    void OnGuiElementMouseClick(GuiElement *f_guiElement, unsigned char f_button, unsigned char f_state);
protected:
    explicit WidgetManager(Core *f_core);
    ~WidgetManager();

    void DoPulse();

    void OnHandActivated(unsigned char f_hand);
    void OnHandDeactivated(unsigned char f_hand);
    void OnButtonPress(unsigned char f_hand, uint32_t f_button);
    void OnButtonRelease(unsigned char f_hand, uint32_t f_button);
    void OnDashboardOpen();
    void OnDashboardClose();

    friend Core;
};
