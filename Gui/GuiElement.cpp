#include "stdafx.h"

#include "Gui/GuiElement.h"
#include "Gui/GuiStructures.h"

GuiElement::GuiElement()
{
    m_userPointer = nullptr;

    m_position = sf::Vector2f(0.f, 0.f);
    m_size = sf::Vector2f(0.f, 0.f);
    m_color = sf::Color(255U, 255U, 255U, 255U);
    m_hoverColor = sf::Color(255U, 255U, 255U, 255U);
    m_activeHover = false;
    m_visible = true;
    m_update = false;

    m_mouseClickCallback = nullptr;
    m_mouseMoveCallback = nullptr;
}
GuiElement::~GuiElement()
{
}

void GuiElement::SetPosition(const sf::Vector2f &f_pos)
{
    if(m_position != f_pos)
    {
        m_position = f_pos;
        m_update = true;
    }
}

void GuiElement::SetSize(const sf::Vector2f &f_size)
{
    if(m_size != f_size)
    {
        m_size = f_size;
        m_update = true;
    }
}

void GuiElement::SetVisibility(bool f_state)
{
    m_visible = f_state;
}

void GuiElement::SetColor(const sf::Color &f_color)
{
    m_color = f_color;
}
void GuiElement::SetHoverColor(const sf::Color &f_color)
{
    m_hoverColor = f_color;
}

void GuiElement::SetMouseClickCallback(const std::function<void(GuiElement*, unsigned char, unsigned char, unsigned int, unsigned int)> &f_callback)
{
    m_mouseClickCallback = f_callback;
}
void GuiElement::RemoveMouseClickCallback()
{
    m_mouseClickCallback = nullptr;
}

void GuiElement::SetMouseMoveCallback(const std::function<void(GuiElement*, unsigned int, unsigned int)> &f_callback)
{
    m_mouseMoveCallback = f_callback;
}
void GuiElement::RemoveMouseMoveCallback()
{
    m_mouseMoveCallback = nullptr;
}

void GuiElement::ProcessMouseClick(unsigned char f_button, unsigned char f_state, unsigned int f_mouseX, unsigned f_mouseY)
{
    if(m_visible)
    {
        const sf::FloatRect l_rectangle(m_position.x, m_position.y, m_size.x, m_size.y);
        if(l_rectangle.contains(static_cast<float>(f_mouseX), static_cast<float>(f_mouseY)))
        {
            ProcessMouseClickInternal(f_button, f_state, f_mouseX, f_mouseY);
            if(m_mouseClickCallback) m_mouseClickCallback(this, f_button, f_state, f_mouseX, f_mouseY);
        }
    }
}

void GuiElement::ProcessMouseMove(unsigned int f_mouseX, unsigned f_mouseY)
{
    if(m_visible)
    {
        const sf::FloatRect l_rectangle(m_position.x, m_position.y, m_size.x, m_size.y);
        if(l_rectangle.contains(static_cast<float>(f_mouseX), static_cast<float>(f_mouseY)))
        {
            m_activeHover = true;
            ProcessMouseMoveInternal(f_mouseX, f_mouseY);
            if(m_mouseMoveCallback) m_mouseMoveCallback(this, f_mouseX, f_mouseY);
        }
        else m_activeHover = false;
    }
}

void GuiElement::Update()
{
    m_update = false;
}
