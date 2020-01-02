#pragma once

class GuiElement
{
    bool m_visible;

    std::function<void(void*, unsigned char, unsigned char, unsigned int, unsigned int)> m_mouseClickCallback;
    std::function<void(void*, unsigned int, unsigned int)> m_mouseMoveCallback;

    GuiElement(const GuiElement &that) = delete;
    GuiElement& operator=(const GuiElement &that) = delete;
public:
    void SetPosition(const sf::Vector2f &f_pos);
    inline const sf::Vector2f& GetPosition() const { return m_position; }

    void SetSize(const sf::Vector2f &f_size);
    inline const sf::Vector2f& GetSize() const { return m_size; }

    void SetVisibility(bool f_state);
    inline bool GetVisibility() const { return m_visible; }

    void SetColor(const sf::Color &f_color);
    void SetHoverColor(const sf::Color &f_color);

    void SetMouseClickCallback(const std::function<void(void*, unsigned char, unsigned char, unsigned int, unsigned int)> &f_callback);
    void RemoveMouseClickCallback();

    void SetMouseMoveCallback(const std::function<void(void*, unsigned int, unsigned int)> &f_callback);
    void RemoveMouseMoveCallback();

    // Internal methods for subclasses
    virtual void ProcessMouseClickInternal(unsigned char f_button, unsigned char f_state, unsigned int f_mouseX, unsigned f_mouseY) {}
    virtual void ProcessMouseMoveInternal(unsigned int f_mouseX, unsigned f_mouseY) {}
protected:
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    bool m_update;
    sf::Color m_hoverColor;
    sf::Color m_color;
    bool m_activeHover;

    GuiElement();
    virtual ~GuiElement();

    virtual void GetDrawables(std::vector<const sf::Drawable*> &f_vec) const = 0;

    virtual void Update();

    void ProcessMouseClick(unsigned char f_button, unsigned char f_state, unsigned int f_mouseX, unsigned f_mouseY);
    void ProcessMouseMove(unsigned int f_mouseX, unsigned f_mouseY);

    friend class GuiSystem;
};
