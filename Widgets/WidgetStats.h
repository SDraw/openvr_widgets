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

        StatsMode_Count
    };
    enum StatsText : size_t
    {
        StatsText_Time = 0U,
        StatsText_Date,
        StatsText_Cpu,
        StatsText_Ram,
        StatsText_Frame,

        StatsText_Count
    };

    sf::RenderTexture *m_renderTexture;
    sf::Font *m_font;
    sf::Text *m_fontText[StatsText_Count];
    sf::Texture *m_texture;
    sf::Sprite *m_spriteIcon;

    struct WinHandles
    {
        PDH_HQUERY m_query;
        PDH_HCOUNTER m_counter;
        MEMORYSTATUSEX m_memoryStatus;
    } m_winHandles;

    ULONGLONG m_lastPressTick;
    std::time_t m_lastTime;
    int m_lastDay;

    size_t m_statsMode;
    bool m_forceUpdate;

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
