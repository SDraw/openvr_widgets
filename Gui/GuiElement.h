#pragma once

class GuiElement
{
    bool m_visible;
    bool m_interactable;

    void *m_userPointer;

    std::function<void(GuiElement*, unsigned char, unsigned char, unsigned int, unsigned int)> m_mouseClickCallback;
    std::function<void(GuiElement*, unsigned int, unsigned int)> m_mouseMoveCallback;

    GuiElement(const GuiElement &that) = delete;
    GuiElement& operator=(const GuiElement &that) = delete;
public:
    virtual ~GuiElement();

    void SetPosition(const sf::Vector2f &f_pos);
    const sf::Vector2f& GetPosition() const;

    void SetSize(const sf::Vector2f &f_size);
    const sf::Vector2f& GetSize() const;

    void SetVisibility(bool f_state);
    bool GetVisibility() const;

    void SetColor(const sf::Color &f_color);
    void SetSelectionColor(const sf::Color &f_color);

    void SetUserPointer(void *f_pointer);
    void* GetUserPointer() const;

    const std::vector<const sf::Drawable*>& GetDrawables() const;

    void SetClickCallback(const std::function<void(GuiElement*, unsigned char, unsigned char, unsigned int, unsigned int)> &f_callback);
    void RemoveClickCallback();
    void ProcessClick(unsigned char f_button, unsigned char f_state, unsigned int f_mouseX, unsigned f_mouseY);

    void SetMoveCallback(const std::function<void(GuiElement*, unsigned int, unsigned int)> &f_callback);
    void RemoveMoveCallback();
    void ProcessMove(unsigned int f_mouseX, unsigned f_mouseY);

    virtual void Update();
protected:
    std::vector<const sf::Drawable*> m_drawables;

    sf::Vector2f m_position;
    sf::Vector2f m_size;
    sf::Color m_selectionColor;
    sf::Color m_color;
    bool m_activeHover;
    bool m_update;

    GuiElement();

    // Internal methods for subclasses
    virtual void ProcessFocusInternal(unsigned int f_mouseX, unsigned int f_mouseY);
    virtual void ProcessBlurInternal(unsigned int f_mouseX, unsigned int f_mouseY);
    virtual void ProcessClickInternal(unsigned char f_button, unsigned char f_state, unsigned int f_mouseX, unsigned int f_mouseY);
    virtual void ProcessMoveInternal(unsigned int f_mouseX, unsigned int f_mouseY);
};
