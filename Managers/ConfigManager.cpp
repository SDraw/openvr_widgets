#include "stdafx.h"
#include "Utils/GlobalStructures.h"

#include "Managers/ConfigManager.h"
#include "Core/Core.h"
#include "Utils/Utils.h"

#include "Core/GlobalSettings.h"

const std::vector<std::string> g_ConfigSettings
{
    "language", "update_rate", "watch_font", "capture_rate"
};
enum ConfigSettingIndex : size_t
{
    CSI_Language = 0U,
    CSI_UpdateRate,
    CSI_WatchFont,
    CSI_CaptureRate
};

const std::vector<std::string> g_ConfigLanguages
{
    "en", "ru"
};

ConfigManager::ConfigManager(Core *f_core)
{
    m_core = f_core;
    m_settingsFile = new pugi::xml_document();
    m_language = ELanguage::ELanguage_English;
    m_updateDelay = 11U; // ~90 FPS by default
    m_statsFont.assign("Hack-Regular.ttf");
    m_captureDelay = 66U; // ~15 FPS by default
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
        pugi::xml_node l_root = m_settingsFile->child("root");
        for(pugi::xml_node l_node = l_root.child("setting"); l_node; l_node = l_node.next_sibling("setting"))
        {
            pugi::xml_attribute l_attribName = l_node.attribute("name");
            pugi::xml_attribute l_attribValue = l_node.attribute("value");
            if(l_attribName && l_attribValue)
            {
                switch(ReadEnumVector(l_attribName.as_string(), g_ConfigSettings))
                {
                    case CSI_Language:
                    {
                        size_t l_langIndex = ReadEnumVector(l_attribValue.as_string("en"), g_ConfigLanguages);
                        if(l_langIndex != std::numeric_limits<size_t>::max()) m_language = static_cast<unsigned char>(l_langIndex);
                    } break;
                    case CSI_UpdateRate:
                        m_updateDelay = l_attribValue.as_uint(11U);
                        break;
                    case CSI_WatchFont:
                        m_statsFont.assign(l_attribValue.as_string("Hack-Regular.ttf"));
                        break;
                    case CSI_CaptureRate:
                        m_captureDelay = l_attribValue.as_uint(66U);
                        break;
                }
            }
        }
    }

    GlobalSettings::SetDirectory(m_directory);
    GlobalSettings::SetStatsFont(m_statsFont);
    GlobalSettings::SetCaptureDelay(m_captureDelay);
}

void ConfigManager::Save()
{
    m_settingsFile->save_file("settings.xml");
}
