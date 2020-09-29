#include "stdafx.h"

#include "Gui/GuiButton.h"

GuiButton::GuiButton(const sf::Font *f_font)
{
    m_fontText = new sf::Text("", *f_font, 14U);
    m_background = new sf::RectangleShape();
    m_size = { 64.f, 64.f };
    m_backgroundColor = { 127U, 127U, 127U, 255U };

    m_drawables.push_back(m_background);
    m_drawables.push_back(m_fontText);
}
GuiButton::~GuiButton()
{
    delete m_fontText;
    delete m_background;
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

void GuiButton::SetBackgroundColor(const sf::Color &f_color)
{
    if(m_backgroundColor != f_color)
    {
        m_backgroundColor = f_color;
        m_update = true;
    }
}

void GuiButton::ProcessFocusInternal(unsigned int f_mouseX, unsigned int f_mouseY)
{
    m_background->setFillColor(m_selectionColor);

    GuiElement::ProcessFocusInternal(f_mouseX, f_mouseY);
}
void GuiButton::ProcessBlurInternal(unsigned int f_mouseX, unsigned int f_mouseY)
{
    m_background->setFillColor(m_backgroundColor);

    GuiElement::ProcessBlurInternal(f_mouseX, f_mouseY);
}

void GuiButton::Update()
{
    if(m_update)
    {
        m_background->setPosition(m_position);
        m_background->setSize(m_size);
        m_background->setFillColor(m_activeHover ? m_selectionColor : m_backgroundColor);

        const sf::FloatRect l_bounds = m_fontText->getLocalBounds();
        m_fontText->setPosition(m_position.x + (m_size.x - l_bounds.width)*0.5f, m_position.y + (m_size.y - l_bounds.height)*0.5f);
        m_fontText->setFillColor(m_color);
    }

    GuiElement::Update();
}
