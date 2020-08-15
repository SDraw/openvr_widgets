#pragma once

class Core;

class ConfigManager final
{
    Core *m_core;

    pugi::xml_document *m_settingsFile;
    std::string m_directory;
    unsigned int m_updateDelay;
    std::string m_guiFont;

    ConfigManager(const ConfigManager &that) = delete;
    ConfigManager& operator=(const ConfigManager &that) = delete;
public:
    inline const std::string& GetDirectory() const { return m_directory; }
    inline unsigned int GetUpdateDelay() { return m_updateDelay; }
    inline const std::string& GetGuiFont() const { return m_guiFont; }
protected:
    explicit ConfigManager(Core *f_core);
    ~ConfigManager();

    void Load();
    void Save();

    friend class Core;
};
