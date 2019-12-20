#include "stdafx.h"

#include "Gui/GuiButton.h"

#include "Core/GlobalSettings.h"

GuiButton::GuiButton(const sf::Texture *f_texture, const sf::Font *f_font)
{
    m_sprite = new sf::Sprite(*f_texture);
    m_fontText = new sf::Text("", *f_font, 14U);

    const sf::Vector2u l_textureSize = f_texture->getSize();
    m_size.x = static_cast<float>(l_textureSize.x);
    m_size.y = static_cast<float>(l_textureSize.y);
}
GuiButton::~GuiButton()
{
    delete m_sprite;
    delete m_fontText;
}

void GuiButton::SetText(const std::string &f_string)
{
    m_fontText->setString(f_string);
}
void GuiButton::SetTextSize(unsigned int f_size)
{
    m_fontText->setCharacterSize(f_size);
}

void GuiButton::GetDrawables(std::vector<const sf::Drawable*> &f_vec) const
{
    f_vec.push_back(m_sprite);
    f_vec.push_back(m_fontText);
}

void GuiButton::Update()
{
    const sf::Texture *f_texture = m_sprite->getTexture();
    if(f_texture)
    {
        sf::Vector2u l_textureSize = f_texture->getSize();
        m_sprite->setScale(m_size.x / static_cast<float>(l_textureSize.x), m_size.y / static_cast<float>(l_textureSize.y));
    }
    m_sprite->setPosition(m_position);
    m_sprite->setColor(m_activeHover ? m_hoverColor : m_color);

    sf::FloatRect l_bounds = m_fontText->getLocalBounds();
    m_fontText->setPosition(m_position.x + (m_size.x - l_bounds.width)*0.5f, m_position.y + (m_size.y - l_bounds.height)*0.5f);
    m_fontText->setFillColor(m_activeHover ? m_hoverColor : m_color);
}
