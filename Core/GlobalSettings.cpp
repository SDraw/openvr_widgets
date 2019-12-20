#include "stdafx.h"
#include "Core/GlobalSettings.h"

std::string GlobalSettings::ms_directory;
unsigned int GlobalSettings::ms_captureDelay = 66U;
std::string GlobalSettings::ms_guiFont;
std::string GlobalSettings::ms_guiButton;

void GlobalSettings::SetDirectory(const std::string &f_dir)
{
    ms_directory.assign(f_dir);
}
void GlobalSettings::SetCaptureDelay(unsigned int f_delay)
{
    ms_captureDelay = f_delay;
}
void GlobalSettings::SetGuiFont(const std::string &f_font)
{
    ms_guiFont.assign(f_font);
}
void GlobalSettings::SetGuiButton(const std::string &f_button)
{
    ms_guiButton.assign(f_button);
}
