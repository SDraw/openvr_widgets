#include "stdafx.h"

#include "Widgets/WidgetStats.h"
#include "Utils/Transformation.h"

#include "Core/GlobalSettings.h"
#include "Core/VRTransform.h"
#include "Utils/GlobalStructures.h"
#include "Utils/Utils.h"

extern const float g_Pi;
extern const glm::vec3 g_AxisZN;
extern const sf::Color g_ClearColor;

const sf::Vector2f g_RenderTargetSize(512.f, 128.f);
const glm::vec3 g_OverlayOffset(0.f, 0.05f, 0.f);
const glm::vec2 g_ViewAngleRange(g_Pi / 6.f, g_Pi / 12.f);
const float g_ViewAngleRangeDif = (g_ViewAngleRange.x - g_ViewAngleRange.y);

const char* g_WeekDayEn[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
const wchar_t* g_WeekDayRu[] = {
    L"\u0412\u0441", L"\u041f\u043d", L"\u0412\u0442", L"\u0421\u0440", L"\u0427\u0442", L"\u041f\u0442", L"\u0421\u0431"
};

const char* g_MemorySizeEn = "MB";
const wchar_t* g_MemorySizeRu = L"\u041C\u0411";

WidgetStats::WidgetStats()
{
    m_renderTexture = nullptr;
    m_font = nullptr;
    m_fontTextTime = nullptr;
    m_fontTextDate = nullptr;
    m_fontTextCpu = nullptr;
    m_fontTextRam = nullptr;
    m_textureWatch = nullptr;
    m_textureCpu = nullptr;
    m_textureRam = nullptr;
    m_spriteIcon = nullptr;
    m_lastTime = 0U;
    m_lastDay = -1;
    m_lastPressTick = 0U;
    m_winHandles.m_query = NULL;
    m_winHandles.m_counter = NULL;
    m_winHandles.m_memoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
    m_forceUpdate = false;
    m_statsMode = SM_Watch;
}
WidgetStats::~WidgetStats()
{
    Cleanup();
}

void WidgetStats::Cleanup()
{
    if(m_winHandles.m_query != NULL)
    {
        PdhCloseQuery(m_winHandles.m_query);
        m_winHandles.m_query = NULL;
    }

    if(m_overlayHandle != vr::k_ulOverlayHandleInvalid)
    {
        ms_vrOverlay->HideOverlay(m_overlayHandle);
        ms_vrOverlay->ClearOverlayTexture(m_overlayHandle);
        ms_vrOverlay->DestroyOverlay(m_overlayHandle);
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    }

    delete m_fontTextTime;
    m_fontTextTime = nullptr;
    delete m_fontTextDate;
    m_fontTextDate = nullptr;
    delete m_fontTextCpu;
    m_fontTextCpu = nullptr;
    delete m_fontTextRam;
    m_fontTextRam = nullptr;

    delete m_spriteIcon;
    m_spriteIcon = nullptr;

    delete m_textureWatch;
    m_textureWatch = nullptr;
    delete m_textureCpu;
    m_textureCpu = nullptr;
    delete m_textureRam;
    m_textureRam = nullptr;

    delete m_font;
    m_font = nullptr;

    delete m_renderTexture;
    m_renderTexture = nullptr;

    m_statsMode = SM_Watch;
    m_valid = false;
}

bool WidgetStats::Create()
{
    if(!m_valid)
    {
        if(PdhOpenQuery(NULL, NULL, &m_winHandles.m_query) == ERROR_SUCCESS)
        {
            PdhAddEnglishCounter(m_winHandles.m_query, L"\\Processor(_Total)\\% Processor Time", NULL, &m_winHandles.m_counter);

            if(ms_vrOverlay->CreateOverlay("ovrw.stats.main", "OpenVR Widget - Stats - Main", &m_overlayHandle) == vr::VROverlayError_None)
            {
                ms_vrOverlay->SetOverlayWidthInMeters(m_overlayHandle, 0.125f);
                ms_vrOverlay->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_SortWithNonSceneOverlays, true);

                m_renderTexture = new sf::RenderTexture();
                if(m_renderTexture->create(static_cast<unsigned int>(g_RenderTargetSize.x), static_cast<unsigned int>(g_RenderTargetSize.y)))
                {
                    m_font = new sf::Font();
                    if(m_font->loadFromFile(GlobalSettings::GetStatsFont()))
                    {
                        m_fontTextTime = new sf::Text("", *m_font, 72U);
                        m_fontTextDate = new sf::Text("", *m_font, 36U);
                        m_fontTextCpu = new sf::Text("", *m_font, 64U);
                        m_fontTextRam = new sf::Text("", *m_font, 40U);

                        m_textureWatch = new sf::Texture();
                        m_textureWatch->loadFromFile("icons/time.png");
                        m_textureCpu = new sf::Texture();
                        m_textureCpu->loadFromFile("icons/chip.png");
                        m_textureRam = new sf::Texture();
                        m_textureRam->loadFromFile("icons/ram.png");

                        m_spriteIcon = new sf::Sprite(*m_textureWatch);
                        m_spriteIcon->setScale(0.75f, 0.75f);
                        m_spriteIcon->setPosition(16.f, 16.f);

                        m_vrTexture.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(m_renderTexture->getTexture().getNativeHandle()));
                        m_vrTexture.eType = vr::TextureType_OpenGL;
                        m_vrTexture.eColorSpace = vr::ColorSpace_Gamma;

                        m_valid = (ms_vrOverlay->SetOverlayTexture(m_overlayHandle, &m_vrTexture) == vr::VROverlayError_None);
                    }
                }
            }
        }
    }
    return m_valid;
}
void WidgetStats::Destroy()
{
    Cleanup();
}

void WidgetStats::Update()
{
    if(m_valid && m_visible)
    {
        std::time_t l_time = std::time(nullptr);
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
                    m_fontTextTime->setString(l_string);

                    sf::FloatRect l_bounds = m_fontTextTime->getLocalBounds();
                    sf::Vector2f l_position(56.f + (g_RenderTargetSize.x - l_bounds.width) * 0.5f, (g_RenderTargetSize.y - l_bounds.height) * 0.5f - 40.f);
                    m_fontTextTime->setPosition(l_position);

                    if(m_lastDay != l_tmTime.tm_yday)
                    {
                        m_lastDay = l_tmTime.tm_yday;

                        sf::String l_sfString;
                        l_string.assign(" ");
                        switch(m_language)
                        {
                            case ELanguage::ELanguage_English:
                            {
                                l_sfString = g_WeekDayEn[l_tmTime.tm_wday];
                                l_string.append(std::to_string(l_tmTime.tm_mon + 1));
                                l_string.push_back('/');
                                l_string.append(std::to_string(l_tmTime.tm_mday));
                            } break;
                            case ELanguage::ELanguage_Russian:
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
                        m_fontTextDate->setString(l_sfString);

                        l_bounds = m_fontTextDate->getLocalBounds();
                        l_position.x = 56.f + (g_RenderTargetSize.x - l_bounds.width) * 0.5f;
                        l_position.y = (g_RenderTargetSize.y - l_bounds.height) * 0.5f + 30.f;
                        m_fontTextDate->setPosition(l_position);
                    }

                    m_renderTexture->draw(*m_fontTextTime);
                    m_renderTexture->draw(*m_fontTextDate);
                } break;

                case SM_Cpu:
                {
                    PDH_FMT_COUNTERVALUE l_counterVal;
                    PdhCollectQueryData(m_winHandles.m_query);
                    PdhGetFormattedCounterValue(m_winHandles.m_counter, PDH_FMT_DOUBLE, NULL, &l_counterVal);

                    std::string l_text(16U, '\0');
                    int l_length = sprintf_s(const_cast<char*>(l_text.data()), 16U, "%.2f", l_counterVal.doubleValue);
                    l_text.resize(static_cast<size_t>(l_length));
                    l_text.push_back('%');
                    m_fontTextCpu->setString(l_text);

                    sf::FloatRect l_bounds = m_fontTextCpu->getLocalBounds();
                    sf::Vector2f l_position(56.f + (g_RenderTargetSize.x - l_bounds.width) * 0.5f, (g_RenderTargetSize.y - l_bounds.height) * 0.5f - 15.f);
                    m_fontTextCpu->setPosition(l_position);

                    m_renderTexture->draw(*m_fontTextCpu);
                } break;

                case SM_Ram:
                {
                    GlobalMemoryStatusEx(&m_winHandles.m_memoryStatus);

                    std::string l_text;
                    l_text.append(std::to_string((m_winHandles.m_memoryStatus.ullTotalPhys - m_winHandles.m_memoryStatus.ullAvailPhys) / 1048576));
                    l_text.push_back('/');
                    l_text.append(std::to_string(m_winHandles.m_memoryStatus.ullTotalPhys / 1048576));
                    l_text.push_back(' ');

                    sf::String l_sfString(l_text);
                    switch(m_language)
                    {
                        case ELanguage::ELanguage_English:
                            l_sfString += g_MemorySizeEn;
                            break;
                        case ELanguage::ELanguage_Russian:
                            l_sfString += g_MemorySizeRu;
                            break;
                    }
                    m_fontTextRam->setString(l_sfString);

                    sf::FloatRect l_bounds = m_fontTextRam->getLocalBounds();
                    sf::Vector2f l_position(56.f + (g_RenderTargetSize.x - l_bounds.width) * 0.5f, (g_RenderTargetSize.y - l_bounds.height) * 0.5f - 5.f);
                    m_fontTextRam->setPosition(l_position);

                    m_renderTexture->draw(*m_fontTextRam);
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
        glm::vec3 l_toHandDir = m_transform->GetPosition() - l_hmdPos;
        l_toHandDir = glm::normalize(l_toHandDir);

        glm::vec3 l_viewDir = l_hmdRot*g_AxisZN;
        float l_opacity = glm::dot(l_toHandDir, l_viewDir);
        l_opacity = glm::acos(l_opacity);
        l_opacity = glm::clamp(l_opacity, g_ViewAngleRange.y, g_ViewAngleRange.x);
        l_opacity = 1.f - ((l_opacity - g_ViewAngleRange.y) / g_ViewAngleRangeDif);
        ms_vrOverlay->SetOverlayAlpha(m_overlayHandle, l_opacity);

        // Set rotation based on direction to HMD
        glm::quat l_rot;
        GetRotationToPoint(l_hmdPos, m_transform->GetPosition(), l_hmdRot, l_rot);
        m_transform->SetRotation(l_rot);

        m_transform->Update();
        ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseRawAndUncalibrated, &m_transform->GetMatrixVR());

        ms_vrOverlay->SetOverlayTexture(m_overlayHandle, &m_vrTexture);
    }
}

void WidgetStats::OnHandDeactivated(unsigned char f_hand)
{
    if(m_valid && m_visible)
    {
        if(f_hand == EVRHand::EVRHand_Right)
        {
            m_visible = false;
            ms_vrOverlay->HideOverlay(m_overlayHandle);
        }
    }
}

void  WidgetStats::OnButtonPress(unsigned char f_hand, uint32_t f_button)
{
    if(m_valid)
    {
        if(f_hand == EVRHand::EVRHand_Right)
        {
            switch(f_button)
            {
                case vr::k_EButton_Grip:
                {
                    ULONGLONG l_tick = GetTickCount64();
                    if((l_tick - m_lastPressTick) <= 300U)
                    {
                        m_visible = true;
                        ms_vrOverlay->ShowOverlay(m_overlayHandle);
                    }
                    m_lastPressTick = l_tick;
                } break;
                case vr::k_EButton_SteamVR_Trigger:
                {
                    if(m_visible)
                    {
                        m_statsMode += 1U;
                        m_statsMode %= SM_Max;
                        switch(m_statsMode)
                        {
                            case SM_Watch:
                                m_spriteIcon->setTexture(*m_textureWatch, true);
                                break;
                            case SM_Cpu:
                                m_spriteIcon->setTexture(*m_textureCpu, true);
                                break;
                            case SM_Ram:
                                m_spriteIcon->setTexture(*m_textureRam, true);
                                break;
                        }
                        m_forceUpdate = true;
                    }
                } break;
            }
        }
    }
}
void WidgetStats::OnButtonRelease(unsigned char f_hand, uint32_t f_button)
{
    if(m_valid && m_visible)
    {
        if((f_hand == EVRHand::EVRHand_Right) && (f_button == vr::k_EButton_Grip))
        {
            m_visible = false;
            ms_vrOverlay->HideOverlay(m_overlayHandle);
        }
    }
}
