#pragma once
class GlobalSettings final
{
    static std::string ms_directory;
    static unsigned int ms_captureDelay;
    static std::string ms_guiFont;
    static std::string ms_guiButton;
public:
    static inline const std::string& GetDirectory() { return ms_directory; }
    static inline unsigned int GetCaptureDelay() { return ms_captureDelay; }
    static inline const std::string& GetGuiFont() { return ms_guiFont; }
protected:
    static void SetDirectory(const std::string &f_dir);
    static void SetCaptureDelay(unsigned int f_delay);
    static void SetGuiFont(const std::string &f_font);
    static void SetGuiButton(const std::string &f_button);

    friend class ConfigManager;
};
