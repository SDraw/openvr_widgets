#include "stdafx.h"

#include "Gui/GuiElement.h"
#include "Gui/GuiStructures.h"

const sf::Color g_DefaultColor(255U, 255U, 255U);
const sf::Color g_DefaultSelectionColor(142U, 205U, 246U);

GuiElement::GuiElement()
{
    m_userPointer = nullptr;

    m_position = sf::Vector2f(0.f, 0.f);
    m_size = sf::Vector2f(0.f, 0.f);
    m_color = g_DefaultColor;
    m_selectionColor = g_DefaultSelectionColor;
    m_activeHover = false;
    m_visible = true;
    m_update = false;

    m_mouseClickCallback = nullptr;
    m_mouseMoveCallback = nullptr;
}
GuiElement::~GuiElement()
{
    m_drawables.clear();
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
    if(m_visible != f_state)
    {
        m_visible = f_state;
        m_update = true;
    }
}

void GuiElement::SetColor(const sf::Color &f_color)
{
    if(m_color != f_color)
    {
        m_color = f_color;
        m_update = true;
    }
}
void GuiElement::SetSelectionColor(const sf::Color &f_color)
{
    if(m_selectionColor != f_color)
    {
        m_selectionColor = f_color;
        m_update = true;
    }
}

void GuiElement::SetClickCallback(const std::function<void(GuiElement*, unsigned char, unsigned char, unsigned int, unsigned int)> &f_callback)
{
    m_mouseClickCallback = f_callback;
}
void GuiElement::RemoveClickCallback()
{
    m_mouseClickCallback = nullptr;
}

void GuiElement::SetMoveCallback(const std::function<void(GuiElement*, unsigned int, unsigned int)> &f_callback)
{
    m_mouseMoveCallback = f_callback;
}
void GuiElement::RemoveMoveCallback()
{
    m_mouseMoveCallback = nullptr;
}

void GuiElement::ProcessClick(unsigned char f_button, unsigned char f_state, unsigned int f_mouseX, unsigned f_mouseY)
{
    if(m_visible)
    {
        const sf::FloatRect l_rectangle(m_position.x, m_position.y, m_size.x, m_size.y);
        if(l_rectangle.contains(static_cast<float>(f_mouseX), static_cast<float>(f_mouseY)))
        {
            ProcessClickInternal(f_button, f_state, f_mouseX, f_mouseY);
            if(m_mouseClickCallback) m_mouseClickCallback(this, f_button, f_state, f_mouseX, f_mouseY);
        }
    }
}
void GuiElement::ProcessClickInternal(unsigned char f_button, unsigned char f_state, unsigned int f_mouseX, unsigned f_mouseY)
{
}

void GuiElement::ProcessMove(unsigned int f_mouseX, unsigned f_mouseY)
{
    if(m_visible)
    {
        const sf::FloatRect l_rectangle(m_position.x, m_position.y, m_size.x, m_size.y);
        if(l_rectangle.contains(static_cast<float>(f_mouseX), static_cast<float>(f_mouseY)))
        {
            if(!m_activeHover)
            {
                m_activeHover = true;
                ProcessFocusInternal(f_mouseX, f_mouseY);
            }
            ProcessMoveInternal(f_mouseX, f_mouseY);
            if(m_mouseMoveCallback) m_mouseMoveCallback(this, f_mouseX, f_mouseY);
        }
        else
        {
            if(m_activeHover)
            {
                m_activeHover = false;
                ProcessBlurInternal(f_mouseX, f_mouseY);
            }
        }
    }
}
void GuiElement::ProcessMoveInternal(unsigned int f_mouseX, unsigned int f_mouseY)
{
}

void GuiElement::ProcessFocusInternal(unsigned int f_mouseX, unsigned int f_mouseY)
{
}
void GuiElement::ProcessBlurInternal(unsigned int f_mouseX, unsigned int f_mouseY)
{
}

void GuiElement::Update()
{
    m_update = false;
}
