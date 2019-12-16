#pragma once

class Core;

class ConfigManager final
{
    Core *m_core;

    pugi::xml_document *m_settingsFile;
    std::string m_directory;
    unsigned char m_language;
    unsigned int m_updateDelay;
    std::string m_statsFont;
    unsigned int m_captureDelay;
public:
    inline const std::string& GetDirectory() const { return m_directory; }
    inline unsigned char GetLanguage() { return m_language; }
    inline unsigned int GetUpdateDelay() { return m_updateDelay; }
    inline const std::string& GetStatsFont() const { return m_statsFont; }
    inline unsigned int GetCaptureDelay() { return m_captureDelay; }
protected:
    explicit ConfigManager(Core *f_core);
    ~ConfigManager();

    void Load();
    void Save();

    friend class Core;
};

