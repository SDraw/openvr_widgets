#include "stdafx.h"

#include "Widgets/WidgetStats.h"
#include "Utils/Transformation.h"

#include "Core/GlobalSettings.h"
#include "Core/VRTransform.h"
#include "Utils/GlobalStructures.h"
#include "Utils/Utils.h"

extern const float g_Pi;
extern const glm::vec3 g_AxisX;
extern const glm::vec3 g_AxisY;
extern const glm::vec3 g_AxisZN;
extern const sf::Color g_ClearColor;
extern const unsigned char g_DummyTextureData[];

const sf::Vector2f g_RenderTargetSize(512.f, 128.f);
const glm::vec3 g_OverlayOffset(0.f, 0.05f, 0.f);
const glm::vec2 g_ViewAngleRange(g_Pi / 6.f, g_Pi / 12.f);
const float g_ViewAngleRangeDiff = (g_ViewAngleRange.x - g_ViewAngleRange.y);

const char* g_WeekDayEn[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
const wchar_t* g_WeekDayRu[] = {
    L"\u0412\u0441", L"\u041f\u043d", L"\u0412\u0442", L"\u0421\u0440", L"\u0427\u0442", L"\u041f\u0442", L"\u0421\u0431"
};

const char* g_MemorySizeEn = "MB";
const wchar_t* g_MemorySizeRu = L"\u041C\u0411";

const sf::IntRect g_SpritesBounds[5U] = {
    { 0, 0, 128, 128 },
    { 128, 0, 128, 128 },
    { 0, 128, 128, 128 },
    { 128, 128, 128, 128 },
    { 256, 0, 128, 128 }
};

WidgetStats::WidgetStats()
{
    m_renderTexture = nullptr;
    m_font = nullptr;
    for(size_t i = 0U; i < ST_Count; i++) m_fontText[i] = nullptr;
    m_textureAtlas = nullptr;
    m_spriteIcon = nullptr;

    m_winHandles.m_query = NULL;
    m_winHandles.m_counter = NULL;
    m_winHandles.m_memoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

    m_lastTime = 0U;
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
        if(PdhOpenQuery(NULL, NULL, &m_winHandles.m_query) == ERROR_SUCCESS)
        {
            PdhAddEnglishCounter(m_winHandles.m_query, L"\\Processor(_Total)\\% Processor Time", NULL, &m_winHandles.m_counter);

            if(ms_vrOverlay->CreateOverlay("ovrw.stats.main", "OpenVR Widget - Stats - Main", &m_overlay) == vr::VROverlayError_None)
            {
                ms_vrOverlay->SetOverlayWidthInMeters(m_overlay, 0.125f);
                ms_vrOverlay->SetOverlayFlag(m_overlay, vr::VROverlayFlags_SortWithNonSceneOverlays, true);

                m_renderTexture = new sf::RenderTexture();
                if(m_renderTexture->create(static_cast<unsigned int>(g_RenderTargetSize.x), static_cast<unsigned int>(g_RenderTargetSize.y)))
                {
                    m_font = new sf::Font();
                    if(m_font->loadFromFile(GlobalSettings::GetGuiFont()))
                    {
                        m_fontText[ST_Time] = new sf::Text("00:00:00", *m_font, 72U);
                        m_fontText[ST_Date] = new sf::Text("Sun 0/0/0", *m_font, 36U);
                        m_fontText[ST_Cpu] = new sf::Text("0.00 %", *m_font, 64U);
                        m_fontText[ST_Ram] = new sf::Text("0/0", *m_font, 40U);
                        m_fontText[ST_Frame] = new sf::Text("0 | 0 FPS", *m_font, 42U);
                        m_fontText[ST_Power] = new sf::Text("0.00 % | 0.00 %", *m_font, 42U);

                        m_textureAtlas = new sf::Texture;
                        if(!m_textureAtlas->loadFromFile("icons/atlas_stats.png")) m_textureAtlas->loadFromMemory(g_DummyTextureData, 16U);

                        m_spriteIcon = new sf::Sprite(*m_textureAtlas);
                        m_spriteIcon->setScale(0.75f, 0.75f);
                        m_spriteIcon->setPosition(16.f, 16.f);
                        m_spriteIcon->setTextureRect(g_SpritesBounds[SM_Watch]);

                        m_texture.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(m_renderTexture->getTexture().getNativeHandle()));
                        m_valid = true;
                    }
                }
            }
        }
    }

    return m_valid;
}

void WidgetStats::Destroy()
{
    if(m_winHandles.m_query != NULL)
    {
        PdhCloseQuery(m_winHandles.m_query);
        m_winHandles.m_query = NULL;
    }

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
        const std::time_t l_time = std::time(nullptr);
        if(m_lastTime != l_time || m_forceUpdate)
        {
            m_lastTime = l_time;
            m_renderTexture->setActive(true);
            m_renderTexture->clear(g_ClearColor);
            m_renderTexture->draw(*m_spriteIcon);

            switch(m_statsMode)
            {
                case SM_Watch:
                {
                    tm l_tmTime;
                    localtime_s(&l_tmTime, &m_lastTime);

                    std::string l_string;
                    if(l_tmTime.tm_hour < 10) l_string.push_back('0');
                    l_string.append(std::to_string(l_tmTime.tm_hour));
                    l_string.push_back(':');
                    if(l_tmTime.tm_min < 10) l_string.push_back('0');
                    l_string.append(std::to_string(l_tmTime.tm_min));
                    l_string.push_back(':');
                    if(l_tmTime.tm_sec < 10) l_string.push_back('0');
                    l_string.append(std::to_string(l_tmTime.tm_sec));
                    m_fontText[ST_Time]->setString(l_string);

                    sf::FloatRect l_bounds = m_fontText[ST_Time]->getLocalBounds();
                    sf::Vector2f l_position(56.f + (g_RenderTargetSize.x - l_bounds.width) * 0.5f, (g_RenderTargetSize.y - l_bounds.height) * 0.5f - 40.f);
                    m_fontText[ST_Time]->setPosition(l_position);

                    if(m_lastDay != l_tmTime.tm_yday)
                    {
                        m_lastDay = l_tmTime.tm_yday;

                        sf::String l_sfString;
                        l_string.assign(" ");
                        switch(m_language)
                        {
                            case LanguageIndex::LI_English:
                            {
                                l_sfString = g_WeekDayEn[l_tmTime.tm_wday];
                                l_string.append(std::to_string(l_tmTime.tm_mon + 1));
                                l_string.push_back('/');
                                l_string.append(std::to_string(l_tmTime.tm_mday));
                            } break;
                            case LanguageIndex::LI_Russian:
                            {
                                l_sfString = g_WeekDayRu[l_tmTime.tm_wday];
                                l_string.append(std::to_string(l_tmTime.tm_mday));
                                l_string.push_back('/');
                                l_string.append(std::to_string(l_tmTime.tm_mon + 1));
                            } break;
                        }
                        l_string.push_back('/');
                        l_string.append(std::to_string(1900 + l_tmTime.tm_year));
                        l_sfString += l_string;
                        m_fontText[ST_Date]->setString(l_sfString);

                        l_bounds = m_fontText[ST_Date]->getLocalBounds();
                        l_position.x = 56.f + (g_RenderTargetSize.x - l_bounds.width) * 0.5f;
                        l_position.y = (g_RenderTargetSize.y - l_bounds.height) * 0.5f + 30.f;
                        m_fontText[ST_Date]->setPosition(l_position);
                    }

                    m_renderTexture->draw(*m_fontText[ST_Time]);
                    m_renderTexture->draw(*m_fontText[ST_Date]);
                } break;

                case SM_Cpu:
                {
                    PDH_FMT_COUNTERVALUE l_counterVal;
                    PdhCollectQueryData(m_winHandles.m_query);
                    PdhGetFormattedCounterValue(m_winHandles.m_counter, PDH_FMT_DOUBLE, NULL, &l_counterVal);

                    std::string l_text;
                    double l_intPart = 0.f;
                    double l_fractPart = std::modf(l_counterVal.doubleValue, &l_intPart);
                    l_text.append(std::to_string(static_cast<int>(l_intPart)));
                    l_text.push_back('.');
                    l_text.append(std::to_string(static_cast<int>(l_fractPart*100.0)));
                    l_text.push_back('%');
                    m_fontText[ST_Cpu]->setString(l_text);

                    const sf::FloatRect l_bounds = m_fontText[ST_Cpu]->getLocalBounds();
                    const sf::Vector2f l_position(56.f + (g_RenderTargetSize.x - l_bounds.width) * 0.5f, (g_RenderTargetSize.y - l_bounds.height) * 0.5f - 15.f);
                    m_fontText[ST_Cpu]->setPosition(l_position);

                    m_renderTexture->draw(*m_fontText[ST_Cpu]);
                } break;

                case SM_Ram:
                {
                    GlobalMemoryStatusEx(&m_winHandles.m_memoryStatus);

                    std::string l_text;
                    l_text.append(std::to_string((m_winHandles.m_memoryStatus.ullTotalPhys - m_winHandles.m_memoryStatus.ullAvailPhys) / 1048576U));
                    l_text.push_back('/');
                    l_text.append(std::to_string(m_winHandles.m_memoryStatus.ullTotalPhys / 1048576U));
                    l_text.push_back(' ');

                    sf::String l_sfString(l_text);
                    switch(m_language)
                    {
                        case LanguageIndex::LI_English:
                            l_sfString += g_MemorySizeEn;
                            break;
                        case LanguageIndex::LI_Russian:
                            l_sfString += g_MemorySizeRu;
                            break;
                    }
                    m_fontText[ST_Ram]->setString(l_sfString);

                    const sf::FloatRect l_bounds = m_fontText[ST_Ram]->getLocalBounds();
                    const sf::Vector2f l_position(56.f + (g_RenderTargetSize.x - l_bounds.width) * 0.5f, (g_RenderTargetSize.y - l_bounds.height) * 0.5f - 5.f);
                    m_fontText[ST_Ram]->setPosition(l_position);

                    m_renderTexture->draw(*m_fontText[ST_Ram]);
                } break;

                case SM_Frame:
                {
                    std::string l_text;
                    vr::Compositor_FrameTiming l_frameTiming[2U];
                    for(size_t i = 0U; i < 2U; i++) l_frameTiming[i].m_nSize = sizeof(vr::Compositor_FrameTiming);
                    if(ms_vrCompositor->GetFrameTiming(&l_frameTiming[0U]))
                    {
                        if(ms_vrCompositor->GetFrameTiming(&l_frameTiming[1U], 1U))
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
                    const sf::Vector2f l_position(56.f + (g_RenderTargetSize.x - l_bounds.width) * 0.5f, (g_RenderTargetSize.y - l_bounds.height) * 0.5f - 5.f);
                    m_fontText[ST_Frame]->setPosition(l_position);

                    m_renderTexture->draw(*m_fontText[ST_Frame]);
                } break;

                case SM_Power:
                {
                    std::string l_text;
                    float l_power = VRTransform::GetLeftHandPower();
                    if(l_power != 0.f)
                    {
                        float l_intPart = 0.f;
                        float l_fractPart = std::modf(l_power, &l_intPart);
                        l_text.append(std::to_string(static_cast<int>(l_intPart)));
                        l_text.push_back('.');
                        l_text.append(std::to_string(static_cast<int>(l_fractPart*10.f)));
                    }
                    else l_text.push_back('*');
                    l_text.append("% | ");

                    l_power = VRTransform::GetRightHandPower();
                    if(l_power != 0.f)
                    {
                        float l_intPart = 0.f;
                        float l_fractPart = std::modf(l_power, &l_intPart);
                        l_text.append(std::to_string(static_cast<int>(l_intPart)));
                        l_text.push_back('.');
                        l_text.append(std::to_string(static_cast<int>(l_fractPart*10.f)));
                    }
                    else l_text.push_back('*');
                    l_text.push_back('%');

                    m_fontText[ST_Power]->setString(l_text);

                    const sf::FloatRect l_bounds = m_fontText[ST_Power]->getLocalBounds();
                    const sf::Vector2f l_position(56.f + (g_RenderTargetSize.x - l_bounds.width) * 0.5f, (g_RenderTargetSize.y - l_bounds.height) * 0.5f - 5.f);
                    m_fontText[ST_Power]->setPosition(l_position);

                    m_renderTexture->draw(*m_fontText[ST_Power]);
                } break;
            }

            m_renderTexture->display();
            m_renderTexture->setActive(false);

            if(m_forceUpdate) m_forceUpdate = false;
        }

        const glm::vec3 &l_hmdPos = VRTransform::GetHmdPosition();
        const glm::quat &l_hmdRot = VRTransform::GetHmdRotation();

        const glm::quat &l_handRot = VRTransform::GetRightHandRotation();
        m_transform->SetPosition(VRTransform::GetRightHandPosition());
        m_transform->Move(l_handRot*g_OverlayOffset);

        // Set opacity based on angle between view direction and hmd to hand direction
        glm::vec3 l_toHandDir = (m_transform->GetPosition() - l_hmdPos);
        l_toHandDir = glm::normalize(l_toHandDir);

        const glm::vec3 l_viewDir = l_hmdRot*g_AxisZN;
        float l_opacity = glm::dot(l_toHandDir, l_viewDir);
        l_opacity = glm::acos(l_opacity);
        l_opacity = glm::clamp(l_opacity, g_ViewAngleRange.y, g_ViewAngleRange.x);
        l_opacity = 1.f - ((l_opacity - g_ViewAngleRange.y) / g_ViewAngleRangeDiff);
        ms_vrOverlay->SetOverlayAlpha(m_overlay, l_opacity);

        // Set rotation based on direction to HMD
        glm::quat l_rot;
        GetRotationToPoint(l_hmdPos, m_transform->GetPosition(), l_hmdRot, l_rot);
        m_transform->SetRotation(l_rot);

        m_transform->Update();
        ms_vrOverlay->SetOverlayTransformAbsolute(m_overlay, vr::TrackingUniverseRawAndUncalibrated, &m_transform->GetMatrixVR());
        ms_vrOverlay->SetOverlayTexture(m_overlay, &m_texture);
    }
}

void WidgetStats::OnHandDeactivated(unsigned char f_hand)
{
    Widget::OnHandDeactivated(f_hand);

    if(m_valid && m_visible)
    {
        if(f_hand == VRHandIndex::VRHI_Right)
        {
            m_visible = false;
            ms_vrOverlay->HideOverlay(m_overlay);
        }
    }
}

void  WidgetStats::OnButtonPress(unsigned char f_hand, uint32_t f_button)
{
    Widget::OnButtonPress(f_hand, f_button);

    if(m_valid)
    {
        if(f_hand == VRHandIndex::VRHI_Right)
        {
            switch(f_button)
            {
                case vr::k_EButton_Grip:
                {
                    const ULONGLONG l_tick = GetTickCount64();
                    if((l_tick - m_lastPressTick) <= 500U)
                    {
                        m_visible = true;
                        ms_vrOverlay->ShowOverlay(m_overlay);
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
void WidgetStats::OnButtonRelease(unsigned char f_hand, uint32_t f_button)
{
    Widget::OnButtonRelease(f_hand, f_button);

    if(m_valid && m_visible)
    {
        if((f_hand == VRHandIndex::VRHI_Right) && (f_button == vr::k_EButton_Grip))
        {
            m_visible = false;
            ms_vrOverlay->HideOverlay(m_overlay);
        }
    }
}
