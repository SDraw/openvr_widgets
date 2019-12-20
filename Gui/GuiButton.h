#pragma once
#include "Gui/GuiElement.h"

class GuiButton final : public GuiElement
{
    sf::Sprite *m_sprite;
    sf::Text *m_fontText;

    GuiButton(const GuiButton &that) = delete;
    GuiButton& operator=(const GuiButton &that) = delete;
public:
    void SetText(const std::string &f_string);
    void SetTextSize(unsigned int f_size);
protected:
    GuiButton(const sf::Texture *f_texture, const sf::Font *f_font);
    ~GuiButton();

    void GetDrawables(std::vector<const sf::Drawable*> &f_vec) const;
    void Update();

    friend class GuiSystem;
};
