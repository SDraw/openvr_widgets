#pragma once

class Config final
{
    static std::string ms_directory;
    static unsigned char ms_language;
    static unsigned int ms_updateRate;
    static std::string ms_watchFont;
    static unsigned int ms_captureRate;
public:
    enum ConfigLanguage : unsigned char
    {
        CL_English = 0U,
        CL_Russian
    };

    static inline const std::string& GetDirectory() { return ms_directory; }
    static inline unsigned char GetLanguage() { return ms_language; }
    static inline unsigned int GetUpdateRate() { return ms_updateRate; }
    static inline const std::string& GetWatchFont() { return ms_watchFont; }
    static inline unsigned int GetCaptureRate() { return ms_captureRate; }
protected:
    static void Load();

    friend class Core;
};

