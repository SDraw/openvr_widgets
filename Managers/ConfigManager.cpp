#include "stdafx.h"

#include "Managers/ConfigManager.h"
#include "Core/Core.h"
#include "Utils/Utils.h"

const std::vector<std::string> g_configSettings
{
    "target_rate", "gui_font", "keyboard_layout"
};
enum ConfigSettingIndex : size_t
{
    CSI_TargetRate = 0U,
    CSI_GuiFont,
    CSI_KeyboardLayout,

    CSI_Count
};

const std::vector<std::string> g_targetRates
{
    "60", "90", "120", "144"
};

std::string ConfigManager::ms_directory;
std::string ConfigManager::ms_guiFont("fonts/Hack-Regular.ttf");
std::string ConfigManager::ms_keyboardLayout("keyboard_en-ru.xml");

ConfigManager::ConfigManager()
{
    m_targetRate = TF_60;
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
                    case ConfigSettingIndex::CSI_TargetRate:
                    {
                        size_t l_target = ReadEnumVector(l_attribValue.as_string("60"), g_targetRates);
                        if(l_target != std::numeric_limits<size_t>::max()) m_targetRate = static_cast<unsigned char>(l_target);
                    } break;
                    case ConfigSettingIndex::CSI_GuiFont:
                        ms_guiFont.assign(l_attribValue.as_string("fonts/Hack-Regular.ttf"));
                        break;
                    case ConfigSettingIndex::CSI_KeyboardLayout:
                        ms_keyboardLayout.assign(l_attribValue.as_string("keyboard_en-ru.xml"));
                        break;
                }
            }
        }
    }
    delete l_document;
}

void ConfigManager::Save()
{
    pugi::xml_document *l_document = new pugi::xml_document();
    pugi::xml_node l_root = l_document->append_child("settings");
    if(l_root)
    {
        for(size_t i = 0U; i < CSI_Count; i++)
        {
            pugi::xml_node l_setting = l_root.append_child("setting");
            if(l_setting)
            {
                pugi::xml_attribute l_attribName = l_setting.append_attribute("name");
                pugi::xml_attribute l_attribValue = l_setting.append_attribute("value");
                if(l_attribName && l_attribValue)
                {
                    l_attribName.set_value(g_configSettings[i].c_str());

                    switch(i)
                    {
                        case CSI_TargetRate:
                            l_attribValue.set_value(g_targetRates[m_targetRate].c_str());
                            break;
                        case CSI_GuiFont:
                            l_attribValue.set_value(ms_guiFont.c_str());
                            break;
                        case CSI_KeyboardLayout:
                            l_attribValue.set_value(ms_keyboardLayout.c_str());
                            break;
                    }
                }
            }
        }

        l_document->save_file("settings.xml");
    }
    delete l_document;
}

unsigned char ConfigManager::GetTargetRate()
{
    return m_targetRate;
}

void ConfigManager::SetTargetRate(unsigned char f_target)
{
    m_targetRate = f_target%TF_Count;
}


// Static
const std::string& ConfigManager::GetDirectory()
{
    return ms_directory;
}

const std::string& ConfigManager::GetGuiFont()
{
    return ms_guiFont;
}

const std::string& ConfigManager::GetKeyboardLayout()
{
    return ms_keyboardLayout;
}
