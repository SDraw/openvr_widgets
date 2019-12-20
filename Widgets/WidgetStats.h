#pragma once
#include "Widgets/Widget.h"

class Transformation;

class WidgetStats final : public Widget
{
    enum StatsMode : size_t
    {
        StatsMode_Watch = 0U,
        StatsMode_Cpu,
        StatsMode_Ram,
        StatsMode_Frame,

        StatsMode_Max
    };
    unsigned char m_statsMode;

    sf::RenderTexture *m_renderTexture;
    sf::Font *m_font;
    sf::Text *m_fontTextTime;
    sf::Text *m_fontTextDate;
    sf::Text *m_fontTextCpu;
    sf::Text *m_fontTextRam;
    sf::Text *m_fontTextFrame;

    sf::Texture *m_texture;
    sf::Sprite *m_spriteIcon;
    sf::IntRect m_spriteRanges[StatsMode_Max];

    ULONGLONG m_lastPressTick;
    std::time_t m_lastTime;
    int m_lastDay;
    bool m_forceUpdate;

    struct WinHandles
    {
        PDH_HQUERY m_query;
        PDH_HCOUNTER m_counter;
        MEMORYSTATUSEX m_memoryStatus;
    } m_winHandles;

    WidgetStats(const WidgetStats &that) = delete;
    WidgetStats& operator=(const WidgetStats &that) = delete;

    // Widget
    bool Create();
    void Destroy();
    void Cleanup();
    void Update();
    bool CloseRequested() const;
    void OnHandDeactivated(unsigned char f_hand);
    void OnButtonPress(unsigned char f_hand, uint32_t f_button);
    void OnButtonRelease(unsigned char f_hand, uint32_t f_button);
protected:
    WidgetStats();
    ~WidgetStats();

    friend class WidgetManager;
};

