#include "stdafx.h"

#include "Gui/GuiSystem.h"
#include "Gui/GuiButton.h"

extern const sf::Color g_ClearColor;
extern const unsigned char g_DummyTextureData[];

GuiSystem::GuiSystem(const sf::Vector2u &f_size)
{
    m_renderTexture = new sf::RenderTexture();
    if(!m_renderTexture->create(f_size.x, f_size.y))
    {
        delete m_renderTexture;
        m_renderTexture = nullptr;
    }

    for(size_t i = 0U; i < GuiTextureType_Max; i++) m_mainTextures[i] = new sf::Texture();
    m_mainFont = new sf::Font();
}
GuiSystem::~GuiSystem()
{
    for(auto l_element : m_guiElements) delete l_element;
    m_guiElements.clear();

    delete m_renderTexture;
}

void GuiSystem::SetButtonsTexture(const std::string &f_path)
{
    m_mainTextures[GuiTextureType_Button]->loadFromFile(f_path);
}
void GuiSystem::SetFont(const std::string &f_path)
{
    m_mainFont->loadFromFile(f_path);
}

GuiButton* GuiSystem::CreateButton()
{
    GuiButton *l_button = new GuiButton(m_mainTextures[GuiTextureType_Button],m_mainFont);
    m_guiElements.push_back(l_button);
    return l_button;
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
            m_renderTexture->clear(g_ClearColor);

            std::vector<const sf::Drawable*> l_drawables;
            for(auto l_element : m_guiElements)
            {
                l_element->GetDrawables(l_drawables);
                for(auto l_drawable : l_drawables) m_renderTexture->draw(*l_drawable);
            }

            m_renderTexture->setActive(false);
        }
    }
}

void GuiSystem::ProcessMouseClick(unsigned char f_button, unsigned char f_state, unsigned int f_mouseX, unsigned f_mouseY)
{
    for(auto l_element : m_guiElements) l_element->ProcessMouseClick(f_button, f_state, f_mouseX, f_mouseY);
}
void GuiSystem::ProcessMouseMove(unsigned int f_posX, unsigned int f_posY)
{
    for(auto l_element : m_guiElements) l_element->ProcessMouseMove(f_posX, f_posY);
}
