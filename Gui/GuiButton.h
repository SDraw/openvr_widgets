#pragma once
#include "Gui/GuiElement.h"

class GuiButton final : public GuiElement
{
    enum ButtonPart : size_t
    {
        ButtonPart_TopLeft = 0U,
        ButtonPart_TopCenter,
        ButtonPart_TopRight,
        ButtonPart_CenterLeft,
        ButtonPart_CenterCenter,
        ButtonPart_CenterRight,
        ButtonPart_BottomLeft,
        ButtonPart_BottomCenter,
        ButtonPart_BottomRight,

        ButtonPart_Max
    };
    sf::Sprite *m_sprites[ButtonPart_Max];
    sf::Text *m_fontText;

    GuiButton(const GuiButton &that) = delete;
    GuiButton& operator=(const GuiButton &that) = delete;
public:
    void SetText(const char *f_string);
    void SetText(const wchar_t *f_string);
    void SetText(const std::string &f_string);
    void SetText(const std::wstring &f_string);
    void SetTextSize(unsigned int f_size);
protected:
    GuiButton(const sf::Texture *f_texture, const sf::Font *f_font);
    ~GuiButton();

    void GetDrawables(std::vector<const sf::Drawable*> &f_vec) const;
    void Update();

    friend class GuiSystem;
};
