#include "stdafx.h"
#include "Utils/GlobalStructures.h"

#include "Managers/ConfigManager.h"
#include "Core/Core.h"
#include "Utils/Utils.h"

#include "Core/GlobalSettings.h"

const std::vector<std::string> g_ConfigSettings
{
    "update_rate", "gui_font"
};
enum ConfigSettingIndex : size_t
{
    CSI_UpdateRate = 0U,
    CSI_GuiFont
};

ConfigManager::ConfigManager(Core *f_core)
{
    m_core = f_core;
    m_settingsFile = new pugi::xml_document();
    m_updateDelay = 11U; // ~90 FPS by default
    m_guiFont.assign("fonts/Hack-Regular.ttf");
}
ConfigManager::~ConfigManager()
{
    delete m_settingsFile;
}

void ConfigManager::Load()
{
#ifdef _WIN32
    char l_path[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, l_path);
    m_directory.assign(l_path);
#elif __linux__
    char l_result[PATH_MAX];
    ssize_t l_count = readlink( "/proc/self/exe", l_result, PATH_MAX );
    m_directory.assign(l_result,(l_count > 0) ? l_count : 0 );
    m_directory.assign(m_directory.substr(0,m_directory.find_last_of("/\\")));
#endif

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
                    case ConfigSettingIndex::CSI_UpdateRate:
                        m_updateDelay = l_attribValue.as_uint(11U);
                        break;
                    case ConfigSettingIndex::CSI_GuiFont:
                        m_guiFont.assign(l_attribValue.as_string("fonts/Hack-Regular.ttf"));
                        break;
                }
            }
        }
    }

    GlobalSettings::SetDirectory(m_directory);
    GlobalSettings::SetGuiFont(m_guiFont);
}

void ConfigManager::Save()
{
    m_settingsFile->save_file("settings.xml");
}
