#pragma once
class GlobalSettings final
{
    static std::string ms_directory;
    static std::string ms_statsFont;
    static unsigned int ms_captureDelay;
public:
    static inline const std::string& GetDirectory() { return ms_directory; }
    static inline const std::string& GetStatsFont() { return ms_statsFont; }
    static inline unsigned int GetCaptureDelay() { return ms_captureDelay; }
protected:
    static void SetDirectory(const std::string &f_dir);
    static void SetStatsFont(const std::string &f_font);
    static void SetCaptureDelay(unsigned int f_delay);

    friend class ConfigManager;
};
