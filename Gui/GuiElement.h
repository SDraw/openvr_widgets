#pragma once

class GuiElement
{
    bool m_visible;

    void *m_userPointer;

    std::function<void(GuiElement*, unsigned char, unsigned char, unsigned int, unsigned int)> m_mouseClickCallback;
    std::function<void(GuiElement*, unsigned int, unsigned int)> m_mouseMoveCallback;

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
    void SetSelectionColor(const sf::Color &f_color);

    inline void SetUserPointer(void *f_pointer) { m_userPointer = f_pointer; }
    inline void* GetUserPointer() const { return m_userPointer; }

    void SetClickCallback(const std::function<void(GuiElement*, unsigned char, unsigned char, unsigned int, unsigned int)> &f_callback);
    void RemoveClickCallback();

    void SetMoveCallback(const std::function<void(GuiElement*, unsigned int, unsigned int)> &f_callback);
    void RemoveMoveCallback();

    // Internal methods for subclasses
    virtual void ProcessFocusInternal(unsigned int f_mouseX, unsigned int f_mouseY);
    virtual void ProcessBlurInternal(unsigned int f_mouseX, unsigned int f_mouseY);
    virtual void ProcessClickInternal(unsigned char f_button, unsigned char f_state, unsigned int f_mouseX, unsigned int f_mouseY);
    virtual void ProcessMoveInternal(unsigned int f_mouseX, unsigned int f_mouseY);
protected:
    std::vector<const sf::Drawable*> m_drawables;

    sf::Vector2f m_position;
    sf::Vector2f m_size;
    sf::Color m_selectionColor;
    sf::Color m_color;
    bool m_activeHover;
    bool m_update;

    GuiElement();
    virtual ~GuiElement();

    inline const std::vector<const sf::Drawable*>& GetDrawables() const { return m_drawables; }

    virtual void Update();

    void ProcessClick(unsigned char f_button, unsigned char f_state, unsigned int f_mouseX, unsigned f_mouseY);
    void ProcessMove(unsigned int f_mouseX, unsigned f_mouseY);

    friend class GuiSystem;
};
