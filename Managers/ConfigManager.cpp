#include "stdafx.h"
#include "Utils/GlobalStructures.h"

#include "Managers/ConfigManager.h"
#include "Core/Core.h"
#include "Utils/Utils.h"

#include "Core/GlobalSettings.h"

const std::vector<std::string> g_ConfigSettings
{
    "language", "update_rate",
    "gui_font", "gui_button"
};
enum ConfigSetting : size_t
{
    ConfigSetting_Language = 0U,
    ConfigSetting_UpdateRate,
    ConfigSetting_GuiFont,
    ConfigSetting_GuiButton
};

const std::vector<std::string> g_ConfigLanguages
{
    "en", "ru"
};

ConfigManager::ConfigManager(Core *f_core)
{
    m_core = f_core;
    m_settingsFile = new pugi::xml_document();
    m_language = Language::Language_English;
    m_updateDelay = 11U; // ~90 FPS by default
    m_guiFont.assign("fonts/Hack-Regular.ttf");
    m_guiButton.assign("gui/button.png");
}
ConfigManager::~ConfigManager()
{
    delete m_settingsFile;
}

void ConfigManager::Load()
{
    char l_path[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, l_path);
    m_directory.assign(l_path);

    if(m_settingsFile->load_file("settings.xml"))
    {
        const pugi::xml_node l_root = m_settingsFile->child("settings");
        for(pugi::xml_node l_node = l_root.child("setting"); l_node; l_node = l_node.next_sibling("setting"))
        {
            const pugi::xml_attribute l_attribName = l_node.attribute("name");
            const pugi::xml_attribute l_attribValue = l_node.attribute("value");
            if(l_attribName && l_attribValue)
            {
                switch(ReadEnumVector(l_attribName.as_string(), g_ConfigSettings))
                {
                    case ConfigSetting_Language:
                    {
                        const size_t l_langIndex = ReadEnumVector(l_attribValue.as_string("en"), g_ConfigLanguages);
                        if(l_langIndex != std::numeric_limits<size_t>::max()) m_language = static_cast<unsigned char>(l_langIndex);
                    } break;
                    case ConfigSetting_UpdateRate:
                        m_updateDelay = l_attribValue.as_uint(11U);
                        break;
                    case ConfigSetting_GuiFont:
                        m_guiFont.assign(l_attribValue.as_string("fonts/Hack-Regular.ttf"));
                        break;
                    case ConfigSetting_GuiButton:
                        m_guiButton.assign(l_attribValue.as_string("gui/button.png"));
                        break;
                }
            }
        }
    }

    GlobalSettings::SetDirectory(m_directory);
    GlobalSettings::SetGuiFont(m_guiFont);
    GlobalSettings::SetGuiButton(m_guiButton);
}

void ConfigManager::Save()
{
    m_settingsFile->save_file("settings.xml");
}
