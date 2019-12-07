#pragma once
#include "Widgets/Widget.h"

class WidgetWatch final : public Widget
{
    vr::VROverlayHandle_t m_overlayHandle;
    vr::Texture_t m_texture;

    sf::RenderTexture *m_renderTexture;
    sf::Font *m_font;
    sf::Text *m_fontTextTime;
    sf::Text *m_fontTextDate;

    std::time_t m_lastTime;
    int m_lastDay;

    ULONGLONG m_lastPressTick;
    bool m_visible;

    glm::vec3 m_position;
    glm::quat m_rotation;
    glm::mat4 m_matrix;
    vr::HmdMatrix34_t m_vrMatrix;

    void Cleanup();

    bool Create();
    void Update();
    void Destroy();

    void OnHandDeactivated(WidgetHand f_hand);
    void OnButtonPress(WidgetHand f_hand, uint32_t f_button);
    void OnButtonRelease(WidgetHand f_hand, uint32_t f_button);

    WidgetWatch(const WidgetWatch &that) = delete;
    WidgetWatch& operator=(const WidgetWatch &that) = delete;
public:
    WidgetWatch();
    ~WidgetWatch();
};

