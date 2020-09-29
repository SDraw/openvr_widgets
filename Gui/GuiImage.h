#pragma once
#include "Gui/GuiElement.h"

class GuiImage : public GuiElement
{
    sf::Sprite *m_sprite;

    // GuiElement
    void ProcessFocusInternal(unsigned int f_mouseX, unsigned int f_mouseY) override;
    void ProcessBlurInternal(unsigned int f_mouseX, unsigned int f_mouseY) override;
    void Update() override;
public:
    explicit GuiImage(const sf::Texture *f_texture);
    ~GuiImage();

    void SetUV(const sf::Vector2i &f_pos, const sf::Vector2i &f_size);
};

