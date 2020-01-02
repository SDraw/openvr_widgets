#include "stdafx.h"
#include "Core/GlobalSettings.h"

std::string GlobalSettings::ms_directory;
std::string GlobalSettings::ms_guiFont;
std::string GlobalSettings::ms_guiButton;

void GlobalSettings::SetDirectory(const std::string &f_dir)
{
    ms_directory.assign(f_dir);
}
void GlobalSettings::SetGuiFont(const std::string &f_font)
{
    ms_guiFont.assign(f_font);
}
void GlobalSettings::SetGuiButton(const std::string &f_button)
{
    ms_guiButton.assign(f_button);
}
