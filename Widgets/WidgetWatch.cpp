#include "stdafx.h"

#include "Widgets/WidgetWatch.h"
#include "Core/Config.h"
#include "Core/TransformStorage.h"
#include "Utils/Utils.h"

extern const float g_Pi;
extern const glm::mat4 g_IdentityMatrix;
extern const glm::vec3 g_EmptyVector;
extern const glm::quat g_EmptyQuat;
extern const glm::vec3 g_AxisY;
extern const glm::vec3 g_AxisZN;

const sf::Color g_ClearColor(0U, 0U, 0U, 127U);
const sf::Vector2f g_RenderTargetSize(256.f, 64.f);
const glm::vec3 g_OverlayOffset(0.f, 0.05f, 0.f);
const glm::vec2 g_ViewAngleRange(g_Pi / 6.f, g_Pi / 12.f);
const float g_ViewAngleRangeDif = g_ViewAngleRange.x - g_ViewAngleRange.y;

const char* g_WeekDayEng[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
const wchar_t* g_WeekDayRus[] = {
    L"Âñ", L"Ïí", L"Âò", L"Ñð", L"×ò", L"Ïò", L"Ñá"
};

WidgetWatch::WidgetWatch()
{
    m_renderTexture = nullptr;
    m_font = nullptr;
    m_fontTextTime = nullptr;
    m_fontTextDate = nullptr;
    m_lastTime = 0U;
    m_lastDay = -1;
    m_visible = false;
    m_lastPressTick = 0U;
    m_position = g_EmptyVector;
    m_rotation = g_EmptyQuat;
    m_matrix = g_IdentityMatrix;
}
WidgetWatch::~WidgetWatch()
{
    Cleanup();
}

void WidgetWatch::Cleanup()
{
    if(m_overlayHandle != vr::k_ulOverlayHandleInvalid)
    {
        ms_vrOverlay->HideOverlay(m_overlayHandle);
        ms_vrOverlay->DestroyOverlay(m_overlayHandle);
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    }

    delete m_fontTextTime;
    m_fontTextTime = nullptr;

    delete m_fontTextDate;
    m_fontTextDate = nullptr;

    delete m_font;
    m_font = nullptr;

    delete m_renderTexture;
    m_renderTexture = nullptr;
}

bool WidgetWatch::Create()
{
    bool l_result = false;
    if(m_overlayHandle == vr::k_ulOverlayHandleInvalid)
    {
        if(ms_vrOverlay->CreateOverlay("openvr_watches_overlay", "OpenVR watches", &m_overlayHandle) == vr::VROverlayError_None)
        {
            ms_vrOverlay->SetOverlayWidthInMeters(m_overlayHandle, 0.125f);
            ms_vrOverlay->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags::VROverlayFlags_SortWithNonSceneOverlays, true);

            m_renderTexture = new sf::RenderTexture();
            if(m_renderTexture->create(static_cast<unsigned int>(g_RenderTargetSize.x), static_cast<unsigned int>(g_RenderTargetSize.y)))
            {
                m_font = new sf::Font();
                if(m_font->loadFromFile(Config::GetWatchFont()))
                {
                    m_fontTextTime = new sf::Text("", *m_font, 36U);
                    m_fontTextDate = new sf::Text("", *m_font, 18U);

                    m_texture.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(m_renderTexture->getTexture().getNativeHandle()));
                    m_texture.eType = vr::TextureType_OpenGL;
                    m_texture.eColorSpace = vr::ColorSpace_Gamma;

                    l_result = (ms_vrOverlay->SetOverlayTexture(m_overlayHandle, &m_texture) == vr::VROverlayError_None);
                }
            }
        }
    }
    return l_result;
}

void WidgetWatch::Update()
{
    if((m_overlayHandle != vr::k_ulOverlayHandleInvalid) && m_visible)
    {
        std::time_t l_time = std::time(nullptr);
        if(m_lastTime != l_time)
        {
            m_lastTime = l_time;
            m_renderTexture->setActive(true);
            m_renderTexture->clear(g_ClearColor);

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
            sf::Vector2f l_position((g_RenderTargetSize.x - l_bounds.width) * 0.5f, (g_RenderTargetSize.y - l_bounds.height) * 0.5f - 20.f);
            m_fontTextTime->setPosition(l_position);

            if(m_lastDay != l_tmTime.tm_yday)
            {
                m_lastDay = l_tmTime.tm_yday;

                sf::String l_dayString;
                l_string.assign(" ");
                switch(Config::GetLanguage())
                {
                    case Config::CL_English:
                    {
                        l_dayString = g_WeekDayEng[l_tmTime.tm_wday];
                        l_string.append(std::to_string(l_tmTime.tm_mon));
                        l_string.push_back('/');
                        l_string.append(std::to_string(l_tmTime.tm_mday));
                    } break;
                    case Config::CL_Russian:
                    {
                        l_dayString = g_WeekDayRus[l_tmTime.tm_wday];
                        l_string.append(std::to_string(l_tmTime.tm_mday));
                        l_string.push_back('/');
                        l_string.append(std::to_string(l_tmTime.tm_mon));
                    } break;
                }
                l_string.push_back('/');
                l_string.append(std::to_string(1900 + l_tmTime.tm_year));
                l_dayString += l_string;
                m_fontTextDate->setString(l_dayString);

                l_bounds = m_fontTextDate->getLocalBounds();
                l_position.x = (g_RenderTargetSize.x - l_bounds.width) * 0.5f;
                l_position.y = (g_RenderTargetSize.y - l_bounds.height) * 0.5f + 15.f;
                m_fontTextDate->setPosition(l_position);
            }

            m_renderTexture->draw(*m_fontTextTime);
            m_renderTexture->draw(*m_fontTextDate);
            m_renderTexture->display();
            m_renderTexture->setActive(false);
        }

        const glm::vec3 &l_hmdPos = TransformStorage::GetHmdPosition();
        const glm::quat &l_hmdRot = TransformStorage::GetHmdRotation();

        const glm::quat &l_handRot = TransformStorage::GetRightHandRotation();
        m_position = TransformStorage::GetRightHandPosition();
        m_position += (l_handRot*g_OverlayOffset);

        // Set opacity based on angle between view direction and hmd to hand direction
        glm::vec3 l_toHandDir = m_position - l_hmdPos;
        l_toHandDir = glm::normalize(l_toHandDir);

        glm::vec3 l_viewDir = l_hmdRot*g_AxisZN;
        float l_opacity = glm::dot(l_toHandDir, l_viewDir);
        l_opacity = glm::acos(l_opacity);
        l_opacity = glm::clamp(l_opacity, g_ViewAngleRange.y, g_ViewAngleRange.x);
        l_opacity = 1.f - ((l_opacity - g_ViewAngleRange.y) / g_ViewAngleRangeDif);
        ms_vrOverlay->SetOverlayAlpha(m_overlayHandle, l_opacity);

        // Set rotation based on direction to HMD
        glm::vec3 l_toHmdDir = -l_toHandDir;
        glm::vec3 l_up = l_hmdRot*g_AxisY;
        glm::vec3 l_crossX = glm::cross(l_up, l_toHmdDir);
        l_crossX = glm::normalize(l_crossX);

        glm::vec3 l_crossY = glm::cross(l_toHmdDir, l_crossX);
        l_crossY = glm::normalize(l_crossY);

        glm::mat3 l_resultRot(l_crossX, l_crossY, l_toHmdDir);
        m_rotation = glm::quat_cast(l_resultRot);

        m_matrix = glm::translate(g_IdentityMatrix, m_position)*glm::mat4_cast(m_rotation);
        ConvertMatrix(m_matrix, m_vrMatrix);
        ms_vrOverlay->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseRawAndUncalibrated, &m_vrMatrix);

        ms_vrOverlay->SetOverlayTexture(m_overlayHandle, &m_texture);
    }
}

void WidgetWatch::Destroy()
{
    Cleanup();
}

void WidgetWatch::OnHandDeactivated(WidgetHand f_hand)
{
    if(m_overlayHandle != vr::k_ulOverlayHandleInvalid)
    {
        if(f_hand == WH_Right && m_visible)
        {
            m_visible = false;
            ms_vrOverlay->HideOverlay(m_overlayHandle);
        }
    }
}

void  WidgetWatch::OnButtonPress(WidgetHand f_hand, uint32_t f_button)
{
    if(m_overlayHandle != vr::k_ulOverlayHandleInvalid)
    {
        if((f_hand == WH_Right) && (f_button == vr::k_EButton_Grip))
        {
            ULONGLONG l_tick = GetTickCount64();
            if((l_tick - m_lastPressTick) <= 300U)
            {
                m_visible = true;
                ms_vrOverlay->ShowOverlay(m_overlayHandle);
            }
            m_lastPressTick = l_tick;
        }
    }
}
void WidgetWatch::OnButtonRelease(WidgetHand f_hand, uint32_t f_button)
{
    if(m_overlayHandle != vr::k_ulOverlayHandleInvalid)
    {
        if((f_hand == WH_Right) && (f_button == vr::k_EButton_Grip) && m_visible)
        {
            m_visible = false;
            ms_vrOverlay->HideOverlay(m_overlayHandle);
        }
    }
}
