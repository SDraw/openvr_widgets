#include "stdafx.h"

#include "Widgets/WidgetStats.h"

#include "Core/VRDevicesStates.h"
#include "Managers/ConfigManager.h"
#include "Utils/VROverlay.h"
#include "Utils/Transformation.h"
#include "Utils/Utils.h"

extern const float g_pi;
extern const glm::vec3 g_axisX;
extern const glm::vec3 g_axisY;
extern const glm::vec3 g_axisZN;
extern const sf::Color g_clearColor;
extern const unsigned char g_dummyTextureData[];

const sf::Vector2f g_renderTargetSize(512.f, 128.f);
const glm::vec3 g_overlayOffset(0.f, 0.05f, 0.f);
const glm::vec2 g_viewAngleRange(g_pi / 6.f, g_pi / 12.f);
const float g_viewAngleRangeDiff = (g_viewAngleRange.x - g_viewAngleRange.y);

const char* g_WeekDays[]
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

const sf::IntRect g_SpritesBounds[] = {
    { 0, 0, 128, 128 },
    { 128, 0, 128, 128 },
    { 0, 128, 128, 128 },
    { 128, 128, 128, 128 },
    { 256, 0, 128, 128 },
    { 384, 0, 128, 128 }
};

WidgetStats::WidgetStats()
{
    m_size.x = 0.125f;
    m_size.y = 0.03125f;

    m_renderTexture = nullptr;
    m_font = nullptr;
    for(size_t i = 0U; i < ST_Count; i++) m_fontText[i] = nullptr;
    m_textureAtlas = nullptr;
    m_spriteIcon = nullptr;

#ifdef _WIN32
    m_winHandles.m_query = NULL;
    m_winHandles.m_counter = NULL;
    m_winHandles.m_memoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
#elif __linux__
    m_memoryTotal = -1;
    m_cpuTickIdle = 0U;
    m_cpuTickTotal = 0U;
#endif

    m_lastSecond = -1;
    m_lastDay = -1;
    m_lastPressTick = 0U;

    m_forceUpdate = false;
    m_statsMode = SM_Watch;
}
WidgetStats::~WidgetStats()
{
}

bool WidgetStats::Create()
{
    if(!m_valid)
    {
#ifdef _WIN32
        if(PdhOpenQuery(NULL, NULL, &m_winHandles.m_query) == ERROR_SUCCESS)
        {
            PdhAddEnglishCounter(m_winHandles.m_query, L"\\Processor(_Total)\\% Processor Time", NULL, &m_winHandles.m_counter);
        }
#elif __linux__
        std::ifstream l_memInfo("/proc/meminfo",std::ios_base::in);
        std::string l_line;
        while(std::getline(l_memInfo,l_line))
        {
            if(sscanf(l_line.c_str(), "MemTotal: %d kB", &m_memoryTotal) == 1)
            {
                m_memoryTotal /= 1024; // Round to MB
                break;
            }
        }
        l_memInfo.close();
#endif
        if(m_overlayMain->Create("ovrw.stats.main", "OpenVR Widget - Stats - Main"))
        {
            m_overlayMain->SetWidth(0.125f);
            m_overlayMain->SetFlag(vr::VROverlayFlags_SortWithNonSceneOverlays, true);
        }

        m_renderTexture = new sf::RenderTexture();
        if(m_renderTexture->create(static_cast<unsigned int>(g_renderTargetSize.x), static_cast<unsigned int>(g_renderTargetSize.y)))
        {
            m_overlayMain->SetTexture(m_renderTexture->getTexture().getNativeHandle());

            m_font = new sf::Font();
            if(m_font->loadFromFile(ConfigManager::GetGuiFont()))
            {
                m_fontText[ST_Time] = new sf::Text("00:00:00", *m_font, 72U);
                m_fontText[ST_Date] = new sf::Text("Sun 0/0/0", *m_font, 36U);
                m_fontText[ST_Cpu] = new sf::Text("0.00 %", *m_font, 64U);
                m_fontText[ST_Ram] = new sf::Text("0/0", *m_font, 40U);
                m_fontText[ST_Frame] = new sf::Text("0 | 0 FPS", *m_font, 42U);
                m_fontText[ST_ControllerPower] = new sf::Text("0.00 % | 0.00 %", *m_font, 42U);
                m_fontText[ST_TrackerPower] = new sf::Text(" 0.00% | 0.00 % | 0.00 %", *m_font, 24U);

                m_textureAtlas = new sf::Texture;
                if(!m_textureAtlas->loadFromFile("icons/atlas_stats.png")) m_textureAtlas->loadFromMemory(g_dummyTextureData, 16U);

                m_spriteIcon = new sf::Sprite(*m_textureAtlas);
                m_spriteIcon->setScale(0.75f, 0.75f);
                m_spriteIcon->setPosition(16.f, 16.f);
                m_spriteIcon->setTextureRect(g_SpritesBounds[SM_Watch]);

                m_valid = true;
            }
        }

        m_valid = (m_valid && m_overlayMain->IsValid());
    }

    return m_valid;
}

void WidgetStats::Destroy()
{
#ifdef _WIN32
    if(m_winHandles.m_query != NULL)
    {
        PdhCloseQuery(m_winHandles.m_query);
        m_winHandles.m_query = NULL;
    }
#endif

    for(size_t i = 0U; i < ST_Count; i++)
    {
        delete m_fontText[i];
        m_fontText[i] = nullptr;
    }

    delete m_spriteIcon;
    m_spriteIcon = nullptr;
    delete m_textureAtlas;
    m_textureAtlas = nullptr;

    delete m_font;
    m_font = nullptr;

    delete m_renderTexture;
    m_renderTexture = nullptr;

    m_statsMode = SM_Watch;

    Widget::Destroy();
}

void WidgetStats::Update()
{
    if(m_valid && m_visible)
    {
        std::time(&m_time);
        std::tm *l_time = std::localtime(&m_time);

        if((m_lastSecond != l_time->tm_sec) || m_forceUpdate)
        {
            m_renderTexture->setActive(true);
            m_renderTexture->clear(g_clearColor);
            m_renderTexture->draw(*m_spriteIcon);

            switch(m_statsMode)
            {
                case SM_Watch:
                {
                    std::string l_text;
                    if(l_time->tm_hour < 10) l_text.push_back('0');
                    l_text.append(std::to_string(l_time->tm_hour));
                    l_text.push_back(':');
                    if(l_time->tm_min < 10) l_text.push_back('0');
                    l_text.append(std::to_string(l_time->tm_min));
                    l_text.push_back(':');
                    if(l_time->tm_sec < 10) l_text.push_back('0');
                    l_text.append(std::to_string(l_time->tm_sec));
                    m_fontText[ST_Time]->setString(l_text);

                    sf::FloatRect l_bounds = m_fontText[ST_Time]->getLocalBounds();
                    sf::Vector2f l_position(56.f + (g_renderTargetSize.x - l_bounds.width) * 0.5f, (g_renderTargetSize.y - l_bounds.height) * 0.5f - 40.f);
                    m_fontText[ST_Time]->setPosition(l_position);

                    if(m_lastDay != l_time->tm_yday)
                    {
                        m_lastDay = l_time->tm_yday;

                        l_text.assign(g_WeekDays[l_time->tm_wday]);
                        l_text.push_back(' ');
                        l_text.append(std::to_string(l_time->tm_mon + 1));
                        l_text.push_back('/');
                        l_text.append(std::to_string(l_time->tm_mday));
                        l_text.push_back('/');
                        l_text.append(std::to_string(1900 + l_time->tm_year));
                        m_fontText[ST_Date]->setString(l_text);

                        l_bounds = m_fontText[ST_Date]->getLocalBounds();
                        l_position.x = 56.f + (g_renderTargetSize.x - l_bounds.width) * 0.5f;
                        l_position.y = (g_renderTargetSize.y - l_bounds.height) * 0.5f + 30.f;
                        m_fontText[ST_Date]->setPosition(l_position);
                    }

                    m_renderTexture->draw(*m_fontText[ST_Time]);
                    m_renderTexture->draw(*m_fontText[ST_Date]);
                } break;

                case SM_Cpu:
                {
                    std::string l_text;
#ifdef _WIN32
                    PDH_FMT_COUNTERVALUE l_counterVal;
                    PdhCollectQueryData(m_winHandles.m_query);
                    PdhGetFormattedCounterValue(m_winHandles.m_counter, PDH_FMT_DOUBLE, NULL, &l_counterVal);

                    double l_intPart = 0.f;
                    double l_fractPart = std::modf(l_counterVal.doubleValue, &l_intPart);
                    l_text.append(std::to_string(static_cast<int>(l_intPart)));
                    l_text.push_back('.');
                    l_text.append(std::to_string(static_cast<int>(l_fractPart*100.0)));
#elif __linux__
                    std::ifstream l_stats("/proc/stat");
                    l_stats.ignore(5U, ' ');
                    std::vector<size_t> l_ticks;
                    for(size_t l_tick = 0U; l_stats >> l_tick;) l_ticks.push_back(l_tick);
                    l_stats.close();

                    size_t l_tickTotal = std::accumulate(l_ticks.begin(),l_ticks.end(),0);
                    float l_resultIdle = static_cast<float>(l_ticks[3U] - m_cpuTickIdle);
                    float l_resultTotal = static_cast<float>(l_tickTotal - m_cpuTickTotal);

                    m_cpuTickIdle = l_ticks[3U];
                    m_cpuTickTotal = l_tickTotal;
                    float l_usage = 100.f*(1.f - l_resultIdle / l_resultTotal);

                    float l_intPart = 0.f;
                    float l_fractPart = std::modf(l_usage, &l_intPart);
                    l_text.append(std::to_string(static_cast<int>(l_intPart)));
                    l_text.push_back('.');
                    l_text.append(std::to_string(static_cast<int>(l_fractPart*100.f)));
#endif
                    l_text.push_back('%');
                    m_fontText[ST_Cpu]->setString(l_text);

                    const sf::FloatRect l_bounds = m_fontText[ST_Cpu]->getLocalBounds();
                    const sf::Vector2f l_position(56.f + (g_renderTargetSize.x - l_bounds.width) * 0.5f, (g_renderTargetSize.y - l_bounds.height) * 0.5f - 15.f);
                    m_fontText[ST_Cpu]->setPosition(l_position);

                    m_renderTexture->draw(*m_fontText[ST_Cpu]);
                } break;

                case SM_Ram:
                {
                    std::string l_text;
#ifdef _WIN32
                    if(GlobalMemoryStatusEx(&m_winHandles.m_memoryStatus))
                    {
                        l_text.append(std::to_string((m_winHandles.m_memoryStatus.ullTotalPhys - m_winHandles.m_memoryStatus.ullAvailPhys) / 1048576U));
                        l_text.push_back('/');
                        l_text.append(std::to_string(m_winHandles.m_memoryStatus.ullTotalPhys / 1048576U));
                    }
                    else l_text.assign("*/*");
#elif __linux__
                    int l_memAvailable = -1;
                    std::ifstream l_memInfo("/proc/meminfo");
                    std::string l_line;
                    while(std::getline(l_memInfo,l_line))
                    {
                        if(sscanf(l_line.c_str(), "MemAvailable: %d kB", &l_memAvailable) == 1)
                        {
                            l_memAvailable /= 1024; // Round to MB
                            break;
                        }
                    }
                    l_memInfo.close();

                    if(m_memoryTotal != -1)
                    {
                        if(l_memAvailable != -1) l_text.append(std::to_string(m_memoryTotal - l_memAvailable));
                        else l_text.push_back('*');
                        l_text.push_back('/');
                        l_text.append(std::to_string(m_memoryTotal));
                    }
                    else l_text.assign("*/*");
#endif
                    l_text.append(" MB");
                    m_fontText[ST_Ram]->setString(l_text);

                    const sf::FloatRect l_bounds = m_fontText[ST_Ram]->getLocalBounds();
                    const sf::Vector2f l_position(56.f + (g_renderTargetSize.x - l_bounds.width) * 0.5f, (g_renderTargetSize.y - l_bounds.height) * 0.5f - 5.f);
                    m_fontText[ST_Ram]->setPosition(l_position);

                    m_renderTexture->draw(*m_fontText[ST_Ram]);
                } break;

                case SM_Frame:
                {
                    std::string l_text;
                    vr::Compositor_FrameTiming l_frameTiming[2U];
                    for(size_t i = 0U; i < 2U; i++) l_frameTiming[i].m_nSize = sizeof(vr::Compositor_FrameTiming);
                    if(vr::VRCompositor()->GetFrameTiming(&l_frameTiming[0U]))
                    {
                        if(vr::VRCompositor()->GetFrameTiming(&l_frameTiming[1U], 1U))
                        {
                            if(l_frameTiming[0U].m_nFrameIndex != l_frameTiming[1U].m_nFrameIndex)
                            {
                                double l_frameDiff = l_frameTiming[0U].m_flSystemTimeInSeconds - l_frameTiming[1U].m_flSystemTimeInSeconds;
                                if(l_frameDiff > .0)
                                {
                                    int l_sceneFramerate = static_cast<int>(1.0 / l_frameDiff);
                                    l_text.append(std::to_string(l_sceneFramerate));
                                }
                                else l_text.push_back('*');
                            }
                            else l_text.push_back('*');
                        }
                        else l_text.push_back('*');
                        l_text.append(" | ");

                        if(l_frameTiming[0U].m_flCompositorUpdateEndMs > 0.f)
                        {
                            int l_compositorFramerate = static_cast<int>(1000.f / l_frameTiming[0U].m_flCompositorUpdateEndMs);
                            l_text.append(std::to_string(l_compositorFramerate));
                        }
                        else l_text.push_back('*');
                        l_text.append(" FPS");
                    }
                    else l_text.assign("* | * FPS");
                    m_fontText[ST_Frame]->setString(l_text);

                    const sf::FloatRect l_bounds = m_fontText[ST_Frame]->getLocalBounds();
                    const sf::Vector2f l_position(56.f + (g_renderTargetSize.x - l_bounds.width) * 0.5f, (g_renderTargetSize.y - l_bounds.height) * 0.5f - 5.f);
                    m_fontText[ST_Frame]->setPosition(l_position);

                    m_renderTexture->draw(*m_fontText[ST_Frame]);
                } break;

                case SM_ControllerPower:
                {
                    std::string l_text;
                    for(size_t i = VRDeviceIndex::VDI_LeftController; i <= VRDeviceIndex::VDI_RightController; i++)
                    {
                        float l_power = VRDevicesStates::GetDevicePower(i);
                        if(l_power != 0.f)
                        {
                            float l_intPart = 0.f;
                            float l_fractPart = std::modf(l_power, &l_intPart);
                            l_text.append(std::to_string(static_cast<int>(l_intPart)));
                            l_text.push_back('.');
                            l_text.append(std::to_string(static_cast<int>(l_fractPart*10.f)));
                            l_text.push_back('%');
                        }
                        else l_text.push_back('*');

                        if(i != VRDeviceIndex::VDI_RightController) l_text.append(" | ");
                    }

                    m_fontText[ST_ControllerPower]->setString(l_text);

                    const sf::FloatRect l_bounds = m_fontText[ST_ControllerPower]->getLocalBounds();
                    const sf::Vector2f l_position(56.f + (g_renderTargetSize.x - l_bounds.width) * 0.5f, (g_renderTargetSize.y - l_bounds.height) * 0.5f - 5.f);
                    m_fontText[ST_ControllerPower]->setPosition(l_position);

                    m_renderTexture->draw(*m_fontText[ST_ControllerPower]);
                } break;

                case SM_TrackerPower:
                {
                    std::string l_text;
                    for(size_t i = VRDeviceIndex::VDI_FirstTracker; i <= VRDeviceIndex::VDI_ThirdTracker; i++)
                    {
                        float l_power = VRDevicesStates::GetDevicePower(i);
                        if(l_power != 0.f)
                        {
                            float l_intPart = 0.f;
                            float l_fractPart = std::modf(l_power, &l_intPart);
                            l_text.append(std::to_string(static_cast<int>(l_intPart)));
                            l_text.push_back('.');
                            l_text.append(std::to_string(static_cast<int>(l_fractPart*10.f)));
                            l_text.push_back('%');
                        }
                        else l_text.push_back('*');

                        if(i != VRDeviceIndex::VDI_ThirdTracker) l_text.append(" | ");
                    }

                    m_fontText[ST_TrackerPower]->setString(l_text);

                    const sf::FloatRect l_bounds = m_fontText[ST_TrackerPower]->getLocalBounds();
                    const sf::Vector2f l_position(56.f + (g_renderTargetSize.x - l_bounds.width) * 0.5f, (g_renderTargetSize.y - l_bounds.height) * 0.5f - 5.f);
                    m_fontText[ST_TrackerPower]->setPosition(l_position);

                    m_renderTexture->draw(*m_fontText[ST_TrackerPower]);
                } break;
            }

            m_renderTexture->display();
            m_renderTexture->setActive(false);

            m_lastSecond = l_time->tm_sec;
            if(m_forceUpdate) m_forceUpdate = false;
        }

        glm::vec3 l_hmdPos;
        glm::quat l_hmdRot;
        VRDevicesStates::GetDevicePosition(VRDeviceIndex::VDI_Hmd, l_hmdPos);
        VRDevicesStates::GetDeviceRotation(VRDeviceIndex::VDI_Hmd, l_hmdRot);

        glm::vec3 l_handPos;
        glm::quat l_handRot;
        VRDevicesStates::GetDevicePosition(VRDeviceIndex::VDI_RightController, l_handPos);
        VRDevicesStates::GetDeviceRotation(VRDeviceIndex::VDI_RightController, l_handRot);
        m_overlayMain->GetTransform()->SetPosition(l_handPos);
        m_overlayMain->GetTransform()->Move(l_handRot*g_overlayOffset);

        // Set opacity based on angle between view direction and hmd to hand direction
        glm::vec3 l_toHandDir = (l_handPos - l_hmdPos);
        l_toHandDir = glm::normalize(l_toHandDir);

        const glm::vec3 l_viewDir = l_hmdRot*g_axisZN;
        float l_opacity = glm::dot(l_toHandDir, l_viewDir);
        l_opacity = glm::acos(l_opacity);
        l_opacity = glm::clamp(l_opacity, g_viewAngleRange.y, g_viewAngleRange.x);
        l_opacity = 1.f - ((l_opacity - g_viewAngleRange.y) / g_viewAngleRangeDiff);
        m_overlayMain->SetAlpha(l_opacity);

        // Set rotation based on direction to HMD
        glm::quat l_rot;
        GetRotationToPoint(l_hmdPos, l_handPos, l_hmdRot, l_rot);
        m_overlayMain->GetTransform()->SetRotation(l_rot);

        m_overlayMain->Update();
    }
}

void WidgetStats::OnHandDeactivated(size_t f_hand)
{
    Widget::OnHandDeactivated(f_hand);

    if(m_valid && m_visible)
    {
        if(f_hand == VRDeviceIndex::VDI_RightController)
        {
            m_visible = false;
            m_overlayMain->Hide();
        }
    }
}

void  WidgetStats::OnButtonPress(size_t f_hand, uint32_t f_button)
{
    Widget::OnButtonPress(f_hand, f_button);

    if(m_valid)
    {
        if(f_hand == VRDeviceIndex::VDI_RightController)
        {
            switch(f_button)
            {
                case vr::k_EButton_Grip:
                {
                    const unsigned long long l_tick = GetTickCount64();
                    if((l_tick - m_lastPressTick) <= 500U)
                    {
                        m_visible = true;
                        m_forceUpdate = true;
                        m_overlayMain->Show();
                    }
                    m_lastPressTick = l_tick;
                } break;
                case vr::k_EButton_SteamVR_Trigger:
                {
                    if(m_visible)
                    {
                        m_statsMode += 1U;
                        m_statsMode %= SM_Count;
                        m_spriteIcon->setTextureRect(g_SpritesBounds[m_statsMode]);
                        m_forceUpdate = true;
                    }
                } break;
            }
        }
    }
}
void WidgetStats::OnButtonRelease(size_t f_hand, uint32_t f_button)
{
    Widget::OnButtonRelease(f_hand, f_button);

    if(m_valid && m_visible)
    {
        if((f_hand == VRDeviceIndex::VDI_RightController) && (f_button == vr::k_EButton_Grip))
        {
            m_visible = false;
            m_overlayMain->Hide();
        }
    }
}
