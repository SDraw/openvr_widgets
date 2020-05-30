#pragma once
#include "Gui/GuiElement.h"

class GuiText : public GuiElement
{
    sf::Text *m_text;
    unsigned char m_alignment;

    // GuiElement
    void Update() override;
public:
    enum GuiTextAlignment : unsigned char
    {
        GTA_Left = 0U,
        GTA_Center
    };

    void Set(const char *f_string);
    void Set(const wchar_t *f_string);
    void Set(const std::string &f_string);
    void Set(const std::wstring &f_string);
    void SetCharactersSize(unsigned int f_size);
    void SetAlignment(unsigned char f_align);
protected:
    explicit GuiText(const sf::Font *f_font);
    ~GuiText();

    friend class GuiSystem;
};
