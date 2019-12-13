#include "stdafx.h"

#include "Core/Config.h"
#include "Utils/Utils.h"

std::string Config::ms_directory;
unsigned char Config::ms_language = CL_English;
unsigned int Config::ms_updateRate = 11U; // ~90 FPS by default
std::string Config::ms_watchFont("Hack-Regular.ttf");
unsigned int Config::ms_captureRate = 66U; // ~15 FPS by default

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

void Config::Load()
{
    char l_path[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, l_path);
    ms_directory.assign(l_path);

    pugi::xml_document *l_document = new pugi::xml_document();
    if(l_document->load_file("settings.xml"))
    {
        pugi::xml_node l_root = l_document->child("root");
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
                        if(l_langIndex != std::numeric_limits<size_t>::max()) ms_language = static_cast<unsigned char>(l_langIndex);
                    } break;
                    case CSI_UpdateRate:
                        ms_updateRate = l_attribValue.as_uint(11U);
                        break;
                    case CSI_WatchFont:
                        ms_watchFont.assign(l_attribValue.as_string("Hack-Regular.ttf"));
                        break;
                    case CSI_CaptureRate:
                        ms_captureRate = l_attribValue.as_uint(66U);
                        break;
                }
            }
        }
    }
    delete l_document;
}
