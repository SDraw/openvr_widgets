#pragma once
#include "Widgets/Widget.h"

class GuiSystem;
class GuiElement;
class GuiButton;
class Transformation;

class WidgetKeyboard : public Widget
{
    enum LayoutState : unsigned char
    {
        LS_Main = 0U,
        LS_Alternative,

        LS_Count
    };

    struct KeyboadKey
    {
        std::string m_text;
        std::string m_altText;
        unsigned int m_code;
    };

    // ISO keyboard (105) QWERTY UK; https://commons.wikimedia.org/wiki/File:ISO_keyboard_(105)_QWERTY_UK.svg
    GuiSystem *m_guiSystem;
    std::vector<GuiButton*> m_guiButtons;

    static sf::Texture *ms_textureAtlas;

    unsigned char m_layoutState;
#ifdef _WIN32
    std::vector<HKL> m_systemLayouts;
#endif

    bool m_activeMove;
    bool m_activePin;
    unsigned long long m_lastTriggerTick;

    void OnGuiElementClick_Keys(GuiElement *f_guiElement, unsigned char f_button, unsigned char f_state);

    // Widget
    bool Create();
    void Destroy() override;
    void Update();
    void OnHandDeactivated(size_t f_hand) override;
    void OnButtonPress(size_t f_hand, uint32_t f_button) override;
    void OnDashboardOpen() override;
    void OnDashboardClose() override;
public:
    WidgetKeyboard();
    ~WidgetKeyboard();

    static void InitStaticResources();
    static void RemoveStaticResources();
};
