#include "stdafx.h"

#include "Gui/GuiButton.h"

const sf::Vector2f g_DefaultButtonSize(64.f, 64.f);
const sf::Vector2f g_DefaultButtonHalfSize = g_DefaultButtonSize*0.5f;
const sf::IntRect g_ButtonsBounds[] = {
    { 0, 0, 16, 16 }, { 16, 0, 32, 16 }, { 48, 0, 16, 16 },
    { 0, 16, 16, 32 }, { 16, 16, 32, 32 }, { 48, 16, 16, 32 },
    { 0, 48, 16, 16 }, { 16, 48, 32, 16 }, { 48, 48, 16, 16 }
};

GuiButton::GuiButton(const sf::Texture *f_texture, const sf::Font *f_font)
{
    for(size_t i = 0U; i < ButtonPart_Max; i++) m_sprites[i] = new sf::Sprite(*f_texture, g_ButtonsBounds[i]);
    m_fontText = new sf::Text("", *f_font, 14U);
    m_size = g_DefaultButtonSize;
}
GuiButton::~GuiButton()
{
    for(size_t i = 0U; i < ButtonPart_Max; i++) delete m_sprites[i];
    delete m_fontText;
}

void GuiButton::SetText(const char *f_string)
{
    m_fontText->setString(f_string);
    m_update = true;
}
void GuiButton::SetText(const wchar_t *f_string)
{
    m_fontText->setString(f_string);
    m_update = true;
}
void GuiButton::SetText(const std::string &f_string)
{
    m_fontText->setString(f_string);
    m_update = true;
}
void GuiButton::SetText(const std::wstring &f_string)
{
    m_fontText->setString(f_string);
    m_update = true;
}

void GuiButton::SetTextSize(unsigned int f_size)
{
    m_fontText->setCharacterSize(f_size);
    m_update = true;
}

void GuiButton::GetDrawables(std::vector<const sf::Drawable*> &f_vec) const
{
    for(size_t i = 0U; i < ButtonPart_Max; i++) f_vec.push_back(m_sprites[i]);
    f_vec.push_back(m_fontText);
}

void GuiButton::Update()
{
    if(m_update)
    {
        const sf::Vector2f l_diff = (m_size - g_DefaultButtonHalfSize);
        const sf::Vector2f l_scale = (l_diff * 0.03125f); // Division by 32

        // Segment 1
        m_sprites[ButtonPart_TopLeft]->setPosition(m_position);

        // Segment 2
        m_sprites[ButtonPart_TopCenter]->setPosition(m_position.x + 16.f, m_position.y);
        m_sprites[ButtonPart_TopCenter]->setScale(l_scale.x, 1.f);

        // Segment 3
        m_sprites[ButtonPart_TopRight]->setPosition(m_position.x + 16.f + l_diff.x, m_position.y);

        // Segment 4
        m_sprites[ButtonPart_CenterLeft]->setPosition(m_position.x, m_position.y + 16.f);
        m_sprites[ButtonPart_CenterLeft]->setScale(1.f, l_scale.y);

        // Segment 5
        m_sprites[ButtonPart_CenterCenter]->setPosition(m_position.x + 16.f, m_position.y + 16.f);
        m_sprites[ButtonPart_CenterCenter]->setScale(l_scale);

        // Segment 6
        m_sprites[ButtonPart_CenterRight]->setPosition(m_position.x + 16.f + l_diff.x, m_position.y + 16.f);
        m_sprites[ButtonPart_CenterRight]->setScale(1.f, l_scale.y);

        // Segment 7
        m_sprites[ButtonPart_BottomLeft]->setPosition(m_position.x, m_position.y + 16.f + l_diff.y);

        // Segment 8
        m_sprites[ButtonPart_BottomCenter]->setPosition(m_position.x + 16.f, m_position.y + 16.f + l_diff.y);
        m_sprites[ButtonPart_BottomCenter]->setScale(l_scale.x, 1.f);

        // Segment 9
        m_sprites[ButtonPart_BottomRight]->setPosition(m_position.x + 16.f + l_diff.x, m_position.y + 16.f + l_diff.y);

        const sf::FloatRect l_bounds = m_fontText->getLocalBounds();
        m_fontText->setPosition(m_position.x + (m_size.x - l_bounds.width)*0.5f, m_position.y + (m_size.y - l_bounds.height)*0.5f);
    }
    m_fontText->setFillColor(m_activeHover ? m_hoverColor : m_color);
    for(size_t i = 0U; i < ButtonPart_Max; i++) m_sprites[i]->setColor(m_activeHover ? m_hoverColor : m_color);

    GuiElement::Update();
}
