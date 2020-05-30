#pragma once

class GuiElement;
class GuiButton;
class GuiImage;
class GuiText;
/*class GuiCheckbox;
class GuiRectangle;*/

class GuiSystem final
{
    sf::RenderTexture *m_renderTexture;
    sf::Font *m_mainFont;

    std::vector<GuiElement*> m_guiElements;

    GuiSystem(const GuiSystem &that) = delete;
    GuiSystem& operator=(const GuiSystem &that) = delete;
public:
    explicit GuiSystem(const sf::Vector2u &f_size);
    ~GuiSystem();

    inline bool IsValid() const { return (m_renderTexture != nullptr); }

    void SetFont(const std::string &f_path);

    GuiButton* CreateButton();
    GuiImage* CreateImage(const sf::Texture *f_texture);
    GuiText* CreateText();
    /*GuiCheckbox* CreateCheckbox();
    GuiRectangle* CreateRectangle();*/
    void Remove(GuiElement *f_element);

    unsigned int GetRenderTextureHandle() const;

    void ProcessClick(unsigned char f_button, unsigned char f_state, unsigned int f_mouseX, unsigned int f_mouseY);
    void ProcessMove(unsigned int f_posX, unsigned int f_posY);

    void Update();
};
