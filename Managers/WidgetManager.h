#pragma once

class Core;
class GuiSystem;
class GuiElement;
class GuiButton;
class Widget;
class VRDashOverlay;

class WidgetManager final
{
    enum GuiElementIndex : size_t
    {
        GEI_Widgets,
        GEI_Widgets_WindowCapture,
        GEI_Widgets_Keyboard,
        GEI_Widgets_Remove,
        GEI_Widgets_Back,
        
        GEI_Devices,
        GEI_Devices_KinectV1,
        GEI_Devices_KinectV2,
        GEI_Devices_LMotionLeft,
        GEI_Devices_LMotionRight,
        GEI_Devices_Back,

        GEI_Settings,
        GEI_Settings_FPS,
        GEI_Settings_ReassignHands,
        GEI_Settings_Back,

        GEI_Close,

        GEI_Count
    };

    Core *m_core;

    VRDashOverlay *m_dashOverlay;
    vr::VREvent_t m_event;

    std::map<unsigned char, Widget*> m_constantWidgets;
    std::vector<Widget*> m_widgets;

    GuiSystem *m_guiSystem;
    GuiButton *m_guiButtons[GEI_Count];
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
