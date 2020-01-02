#pragma once
class GlobalSettings final
{
    static std::string ms_directory;
    static std::string ms_guiFont;
    static std::string ms_guiButton;
public:
    static inline const std::string& GetDirectory() { return ms_directory; }
    static inline const std::string& GetGuiFont() { return ms_guiFont; }
protected:
    static void SetDirectory(const std::string &f_dir);
    static void SetGuiFont(const std::string &f_font);
    static void SetGuiButton(const std::string &f_button);

    friend class ConfigManager;
};
