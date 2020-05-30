#include "stdafx.h"

#include "Gui/GuiImage.h"

GuiImage::GuiImage(const sf::Texture *f_texture)
{
    m_sprite = new sf::Sprite(*f_texture);
    m_drawables.push_back(m_sprite);

    const sf::Vector2u l_textureSize = f_texture->getSize();
    m_size.x = static_cast<float>(l_textureSize.x);
    m_size.y = static_cast<float>(l_textureSize.y);
}

GuiImage::~GuiImage()
{
    delete m_sprite;
}

void GuiImage::SetUV(const sf::Vector2i &f_pos, const sf::Vector2i &f_size)
{
    const sf::IntRect l_rectangle(f_pos,f_size);
    m_sprite->setTextureRect(l_rectangle);
}

void GuiImage::ProcessFocusInternal(unsigned int f_mouseX, unsigned int f_mouseY)
{
    if(m_sprite) m_sprite->setColor(m_selectionColor);

    GuiElement::ProcessFocusInternal(f_mouseX, f_mouseY);
}

void GuiImage::ProcessBlurInternal(unsigned int f_mouseX, unsigned int f_mouseY)
{
    if(m_sprite) m_sprite->setColor(m_color);

    GuiElement::ProcessBlurInternal(f_mouseX, f_mouseY);
}

void GuiImage::Update()
{
    if(m_update)
    {
        m_sprite->setPosition(m_position);

        //const sf::Vector2u l_textureSize = m_sprite->getTexture()->getSize();
        //m_sprite->setScale(m_size.x / static_cast<float>(l_textureSize.x), m_size.y / static_cast<float>(l_textureSize.y));
    }

    GuiElement::Update();
}
