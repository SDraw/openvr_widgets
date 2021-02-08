#pragma once

class ConfigManager final
{
    static std::string ms_directory;
    static std::string ms_guiFont;
    static std::string ms_keyboardLayout;

    unsigned char m_targetRate;

    ConfigManager(const ConfigManager &that) = delete;
    ConfigManager& operator=(const ConfigManager &that) = delete;
public:
    enum TargetFPS : unsigned char
    {
        TF_60 = 0U,
        TF_90,
        TF_120,
        TF_144,

        TF_Count
    };

    ConfigManager();
    ~ConfigManager();

    void Load();
    void Save();

    unsigned char GetTargetRate();
    void SetTargetRate(unsigned char f_target);

    static const std::string& GetDirectory();
    static const std::string& GetGuiFont();
    static const std::string& GetKeyboardLayout();

};
