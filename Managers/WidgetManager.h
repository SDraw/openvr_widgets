#pragma once

class Core;
class Widget;

class WidgetManager final
{
    Core *m_core;
    std::map<unsigned char, Widget*> m_constantWidgets;
    std::vector<Widget*> m_widgets;

    WidgetManager(const WidgetManager &that) = delete;
    WidgetManager& operator=(const WidgetManager &that) = delete;
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

