#pragma once
#include "Widgets/Widget.h"

class Transformation;
class WidgetStats final : public Widget
{
    sf::RenderTexture *m_renderTexture;
    sf::Font *m_font;
    sf::Text *m_fontTextTime;
    sf::Text *m_fontTextDate;
    sf::Text *m_fontTextCpu;
    sf::Text *m_fontTextRam;

    sf::Texture *m_textureWatch;
    sf::Texture *m_textureCpu;
    sf::Texture *m_textureRam;
    sf::Sprite *m_spriteIcon;

    std::time_t m_lastTime;
    int m_lastDay;
    struct WinHandles
    {
        PDH_HQUERY m_query;
        PDH_HCOUNTER m_counter;
        MEMORYSTATUSEX m_memoryStatus;
    } m_winHandles;
    bool m_forceUpdate;

    ULONGLONG m_lastPressTick;

    enum StatsMode : unsigned char
    {
        SM_Watch = 0U,
        SM_Cpu,
        SM_Ram,

        SM_Max
    };
    unsigned char m_statsMode;

    WidgetStats(const WidgetStats &that) = delete;
    WidgetStats& operator=(const WidgetStats &that) = delete;

    // Widget
    bool Create();
    void Update();
    void Destroy();
    void Cleanup();
    void OnHandDeactivated(unsigned char f_hand);
    void OnButtonPress(unsigned char f_hand, uint32_t f_button);
    void OnButtonRelease(unsigned char f_hand, uint32_t f_button);
protected:
    WidgetStats();
    ~WidgetStats();

    friend class WidgetManager;
};

