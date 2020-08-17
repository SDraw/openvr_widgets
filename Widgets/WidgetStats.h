#pragma once
#include "Widgets/Widget.h"

class Transformation;

class WidgetStats final : public Widget
{
    enum StatsMode : size_t
    {
        SM_Watch = 0U,
        SM_Cpu,
        SM_Ram,
        SM_Frame,
        SM_Power,

        SM_Count
    };
    enum StatsText : size_t
    {
        ST_Time = 0U,
        ST_Date,
        ST_Cpu,
        ST_Ram,
        ST_Frame,
        ST_Power,

        ST_Count
    };

    sf::RenderTexture *m_renderTexture;
    sf::Font *m_font;
    sf::Text *m_fontText[ST_Count];
    sf::Texture *m_textureAtlas;
    sf::Sprite *m_spriteIcon;

#ifdef _WIN32
    struct WinHandles
    {
        PDH_HQUERY m_query;
        PDH_HCOUNTER m_counter;
        MEMORYSTATUSEX m_memoryStatus;
    } m_winHandles;
#elif __linux__
    int m_memoryTotal;
    size_t m_cpuTickIdle;
    size_t m_cpuTickTotal;
#endif

    unsigned long long m_lastPressTick;
    std::time_t m_lastTime;
    int m_lastDay;

    size_t m_statsMode;
    bool m_forceUpdate;

    WidgetStats(const WidgetStats &that) = delete;
    WidgetStats& operator=(const WidgetStats &that) = delete;

    // Widget
    bool Create();
    void Destroy() override;
    void Update();
    void OnHandDeactivated(unsigned char f_hand) override;
    void OnButtonPress(unsigned char f_hand, uint32_t f_button) override;
    void OnButtonRelease(unsigned char f_hand, uint32_t f_button) override;
protected:
    WidgetStats();
    ~WidgetStats();

    friend class WidgetManager;
};
