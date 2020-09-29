#pragma once

class ConfigManager final
{
    static std::string ms_directory;
    static std::string ms_guiFont;
    static unsigned int ms_updateDelay;

    ConfigManager(const ConfigManager &that) = delete;
    ConfigManager& operator=(const ConfigManager &that) = delete;
public:
    ConfigManager();
    ~ConfigManager();

    void Load();

    static const std::string& GetDirectory();
    static unsigned int GetUpdateDelay();
    static const std::string& GetGuiFont();
};
