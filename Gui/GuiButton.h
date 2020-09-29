#pragma once
#include "Gui/GuiElement.h"

class GuiButton final : public GuiElement
{
    sf::Text *m_fontText;
    sf::RectangleShape *m_background;

    sf::Color m_backgroundColor;

    GuiButton(const GuiButton &that) = delete;
    GuiButton& operator=(const GuiButton &that) = delete;

    void ProcessFocusInternal(unsigned int f_mouseX, unsigned int f_mouseY) override;
    void ProcessBlurInternal(unsigned int f_mouseX, unsigned int f_mouseY) override;

    // GuiElement
    void Update() override;
public:
    explicit GuiButton(const sf::Font *f_font);
    ~GuiButton();

    void SetText(const char *f_string);
    void SetText(const wchar_t *f_string);
    void SetText(const std::string &f_string);
    void SetText(const std::wstring &f_string);
    void SetTextSize(unsigned int f_size);

    void SetBackgroundColor(const sf::Color &f_color);
};
