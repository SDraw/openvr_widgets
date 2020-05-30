#pragma once
#include "Widgets/Widget.h"

class GuiSystem;
class GuiElement;
class GuiButton;
class Transformation;

class WidgetKeyboard : public Widget
{
    // ISO keyboard (105) QWERTY UK; https://commons.wikimedia.org/wiki/File:ISO_keyboard_(105)_QWERTY_UK.svg
    enum KeyboardKeyIndex : size_t
    {
        // System
        KKI_Esc = 0U, KKI_PrintScreen, KKI_Pause,

        // Functions
        KKI_F1, KKI_F2, KKI_F3, KKI_F4, KKI_F5, KKI_F6, KKI_F7, KKI_F8, KKI_F9, KKI_F10, KKI_F11, KKI_F12,

        // Characters
        KKI_Tilde, KKI_1, KKI_2, KKI_3, KKI_4, KKI_5, KKI_6, KKI_7, KKI_8, KKI_9, KKI_0, KKI_Dash, KKI_Equal,
        KKI_Q, KKI_W, KKI_E, KKI_R, KKI_T, KKI_Y, KKI_U, KKI_I, KKI_O, KKI_P, KKI_BracketLeft, KKI_BracketRight,
        KKI_A, KKI_S, KKI_D, KKI_F, KKI_G, KKI_H, KKI_J, KKI_K, KKI_L, KKI_Colon, KKI_Apostrophe, KKI_Pound,
        KKI_Backslah, KKI_Z, KKI_X, KKI_C, KKI_V, KKI_B, KKI_N, KKI_M, KKI_Comma, KKI_Dot, KKI_Slash,
        KKI_Space,

        // Modifiers
        KKI_RightShift, KKI_LeftShift,
        KKI_RightControl, KKI_LeftControl, KKI_Alt, KKI_AltGr,

        // Editing
        KKI_Backspace, KKI_Insert,
        KKI_Enter, KKI_Delete,
        KKI_NumEnter,

        // Navigation
        KKI_Tab,
        KKI_Home, KKI_PageUp,
        KKI_End, KKI_PageDown,
        KKI_ArrowUp,
        KKI_ArrowLeft, KKI_ArrowDown, KKI_ArrowRight,

        // Numeric
        KKI_NumDivide, KKI_NumMultiply, KKI_NumSubstract,
        KKI_Num7, KKI_Num8, KKI_Num9, KKI_NumAdd,
        KKI_Num4, KKI_Num5, KKI_Num6,
        KKI_Num1, KKI_Num2, KKI_Num3,
        KKI_Num0, KKI_NumDot,

        // Lock
        KKI_ScrollLock,
        KKI_NumLock,
        KKI_CapsLock,

        KKI_Count
    };
    enum ControlIndex : size_t
    {
        CI_Pin = 0U,
        CI_Close,

        CI_Count
    };

    vr::VROverlayHandle_t m_overlayControls[CI_Count];
    Transformation *m_transformControls[CI_Count];

    GuiSystem *m_guiSystem;
    GuiButton *m_guiButtons[KKI_Count];

    static sf::Texture *ms_textureAtlas;
    static vr::Texture_t ms_textureControls;

    bool m_activeMove;
    bool m_activePin;

    ULONGLONG m_lastTriggerTick;

    // Widget
    bool Create();
    void Destroy() override;
    void Update();
    void OnHandDeactivated(unsigned char f_hand) override;
    void OnButtonPress(unsigned char f_hand, uint32_t f_button) override;
    void OnDashboardOpen() override;
    void OnDashboardClose() override;
protected:
    WidgetKeyboard();
    ~WidgetKeyboard();

    void OnGuiElementClick(GuiElement *f_guiElement, unsigned char f_button, unsigned char f_state);

    static void RemoveStaticResources();

    friend class WidgetManager;
};
