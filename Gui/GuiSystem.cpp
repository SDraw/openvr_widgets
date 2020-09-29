#include "stdafx.h"

#include "Gui/GuiSystem.h"
#include "Gui/GuiButton.h"
#include "Gui/GuiImage.h"
#include "Gui/GuiText.h"
#include "Utils/TexturePooler.h"

extern const sf::Color g_clearColor;
extern const unsigned char g_dummyTextureData[];

GuiSystem::GuiSystem(const sf::Vector2u &f_size)
{
    m_renderTexture = TexturePooler::CreateRenderTexture(f_size.x, f_size.y);
    m_mainFont = new sf::Font();
}
GuiSystem::~GuiSystem()
{
    for(auto l_element : m_guiElements) delete l_element;
    m_guiElements.clear();

    if(m_renderTexture) TexturePooler::DeleteRenderTexture(m_renderTexture);
}

void GuiSystem::SetFont(const std::string &f_path)
{
    m_mainFont->loadFromFile(f_path);
}

GuiButton* GuiSystem::CreateButton()
{
    GuiButton *l_button = new GuiButton(m_mainFont);
    m_guiElements.push_back(l_button);
    return l_button;
}

GuiImage* GuiSystem::CreateImage(const sf::Texture *f_texture)
{
    GuiImage *l_image = new GuiImage(f_texture);
    m_guiElements.push_back(l_image);
    return l_image;
}

GuiText* GuiSystem::CreateText()
{
    GuiText *l_text = new GuiText(m_mainFont);
    m_guiElements.push_back(l_text);
    return l_text;
}

void GuiSystem::Remove(GuiElement *f_element)
{
    auto l_searchIter = std::find(m_guiElements.begin(), m_guiElements.end(), f_element);
    if(l_searchIter != m_guiElements.end())
    {
        delete (*l_searchIter);
        m_guiElements.erase(l_searchIter);
    }
}

unsigned int GuiSystem::GetRenderTextureHandle() const
{
    unsigned int l_result = 0U;
    if(m_renderTexture) l_result = m_renderTexture->getTexture().getNativeHandle();
    return l_result;
}

void GuiSystem::Update()
{
    for(auto l_element : m_guiElements) l_element->Update();
    if(m_renderTexture)
    {
        if(m_renderTexture->setActive(true))
        {
            m_renderTexture->clear(g_clearColor);

            for(auto l_element : m_guiElements)
            {
                if(l_element->GetVisibility())
                {
                    for(auto l_drawable : l_element->GetDrawables()) m_renderTexture->draw(*l_drawable);
                }
            }

            m_renderTexture->display();
            m_renderTexture->setActive(false);
        }
    }
}

void GuiSystem::ProcessClick(unsigned char f_button, unsigned char f_state, unsigned int f_mouseX, unsigned f_mouseY)
{
    for(auto l_element : m_guiElements) l_element->ProcessClick(f_button, f_state, f_mouseX, f_mouseY);
}
void GuiSystem::ProcessMove(unsigned int f_posX, unsigned int f_posY)
{
    for(auto l_element : m_guiElements) l_element->ProcessMove(f_posX, f_posY);
}
