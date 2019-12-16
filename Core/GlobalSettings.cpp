#include "stdafx.h"
#include "Core/GlobalSettings.h"

std::string GlobalSettings::ms_directory;
std::string GlobalSettings::ms_statsFont;
unsigned int GlobalSettings::ms_captureDelay = 66U;

void GlobalSettings::SetDirectory(const std::string &f_dir)
{
    ms_directory.assign(f_dir);
}
void GlobalSettings::SetStatsFont(const std::string &f_font)
{
    ms_statsFont.assign(f_font);
}
void GlobalSettings::SetCaptureDelay(unsigned int f_delay)
{
    ms_captureDelay = f_delay;
}
