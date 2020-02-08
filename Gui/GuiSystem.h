#pragma once

class GuiElement;
class GuiButton;
/*class GuiCheckbox;
class GuiText;
class GuiRectangle;
class GuiImage;*/

class GuiSystem final
{
    enum GuiTextureType : size_t
    {
        GuiTextureType_Button = 0U,

        GuiTextureType_Max
    };

    sf::RenderTexture *m_renderTexture;
    sf::Texture *m_mainTextures[GuiTextureType::GuiTextureType_Max];
    sf::Font *m_mainFont;

    std::vector<GuiElement*> m_guiElements;

    GuiSystem(const GuiSystem &that) = delete;
    GuiSystem& operator=(const GuiSystem &that) = delete;
public:
    explicit GuiSystem(const sf::Vector2u &f_size);
    ~GuiSystem();

    inline bool IsValid() const { return (m_renderTexture != nullptr); }

    void SetButtonsTexture(const std::string &f_path);
    void SetFont(const std::string &f_path);

    GuiButton* CreateButton();
    /*GuiCheckbox* CreateCheckbox();
    GuiText* CreateText();
    GuiRectangle* CreateRectangle();
    GuiImage* CreateImage();*/
    void Remove(GuiElement *f_element);

    unsigned int GetRenderTextureHandle() const;

    void ProcessMouseClick(unsigned char f_button, unsigned char f_state, unsigned int f_mouseX, unsigned int f_mouseY);
    void ProcessMouseMove(unsigned int f_posX, unsigned int f_posY);

    void Update();
};
