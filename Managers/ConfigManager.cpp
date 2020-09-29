#include "stdafx.h"

#include "Managers/ConfigManager.h"
#include "Core/Core.h"
#include "Utils/Utils.h"

const std::vector<std::string> g_configSettings
{
    "update_rate", "gui_font"
};
enum ConfigSettingIndex : size_t
{
    CSI_UpdateRate = 0U,
    CSI_GuiFont
};

std::string ConfigManager::ms_directory;
std::string ConfigManager::ms_guiFont("fonts/Hack-Regular.ttf");
unsigned int ConfigManager::ms_updateDelay = 11U;

ConfigManager::ConfigManager()
{
}

ConfigManager::~ConfigManager()
{
}

void ConfigManager::Load()
{
#ifdef _WIN32
    char l_path[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, l_path);
    ms_directory.assign(l_path);
#elif __linux__
    char l_result[PATH_MAX];
    size_t l_count = readlink( "/proc/self/exe", l_result, PATH_MAX );
    ms_directory.assign(l_result,(l_count > 0) ? l_count : 0 );
    ms_directory.assign(ms_directory.substr(0,ms_directory.find_last_of("/\\")));
#endif

    pugi::xml_document *l_document = new pugi::xml_document();
    if(l_document->load_file("settings.xml"))
    {
        const pugi::xml_node l_root = l_document->child("settings");
        for(pugi::xml_node l_node = l_root.child("setting"); l_node; l_node = l_node.next_sibling("setting"))
        {
            const pugi::xml_attribute l_attribName = l_node.attribute("name");
            const pugi::xml_attribute l_attribValue = l_node.attribute("value");
            if(l_attribName && l_attribValue)
            {
                switch(ReadEnumVector(l_attribName.as_string(), g_configSettings))
                {
                    case ConfigSettingIndex::CSI_UpdateRate:
                        ms_updateDelay = l_attribValue.as_uint(11U);
                        break;
                    case ConfigSettingIndex::CSI_GuiFont:
                        ms_guiFont.assign(l_attribValue.as_string("fonts/Hack-Regular.ttf"));
                        break;
                }
            }
        }
    }
    delete l_document;
}

// Static
const std::string& ConfigManager::GetDirectory()
{
    return ms_directory;
}

unsigned int ConfigManager::GetUpdateDelay()
{
    return ms_updateDelay;
}

const std::string& ConfigManager::GetGuiFont()
{
    return ms_guiFont;
}
