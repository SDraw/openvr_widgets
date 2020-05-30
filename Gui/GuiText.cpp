#include "stdafx.h"

#include "Gui/GuiText.h"

GuiText::GuiText(const sf::Font *f_font)
{
    m_text = new sf::Text("", *f_font, 14U);
    m_drawables.push_back(m_text);
    m_alignment = GTA_Left;
}

GuiText::~GuiText()
{
    delete m_text;
}

void GuiText::Set(const char *f_string)
{
    m_text->setString(f_string);
    m_update = true;
}

void GuiText::Set(const wchar_t *f_string)
{
    m_text->setString(f_string);
    m_update = true;
}

void GuiText::Set(const std::string &f_string)
{
    m_text->setString(f_string);
    m_update = true;
}

void GuiText::Set(const std::wstring &f_string)
{
    m_text->setString(f_string);
    m_update = true;
}

void GuiText::SetCharactersSize(unsigned int f_size)
{
    m_text->setCharacterSize(f_size);
    m_update = true;
}

void GuiText::SetAlignment(unsigned char f_align)
{
    m_alignment = f_align;
    m_update = true;
}

void GuiText::Update()
{
    if(m_update)
    {
        switch(m_alignment)
        {
            case GTA_Left:
                m_text->setPosition(m_position);
                break;
            case GTA_Center:
            {
                const sf::FloatRect l_bounds = m_text->getLocalBounds();
                m_text->setPosition(m_position.x - l_bounds.width*0.5f, m_position.y - l_bounds.height*0.5f);
            } break;
        }
        m_text->setFillColor(m_color);
    }

    GuiElement::Update();
}
