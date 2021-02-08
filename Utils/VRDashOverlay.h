#pragma once

class VRDashOverlay final
{
    vr::VROverlayHandle_t m_handle;
    vr::VROverlayHandle_t m_handleThumb;
    vr::Texture_t m_texture;
    vr::Texture_t m_textureThumb;
public:
    VRDashOverlay();
    ~VRDashOverlay();

    bool Create(const std::string &f_key, const std::string &f_name);
    void Destroy();
    bool IsValid() const;

    void SetTexture(unsigned int f_name);
    void SetTexture(const std::string &f_path);
    void SetThumbTexture(unsigned int f_name);
    void SetThumbTexture(const std::string &f_path);
    void SetInputMethod(vr::VROverlayInputMethod f_method);
    void SetFlag(vr::VROverlayFlags f_flag, bool f_state);
    void SetWidth(float f_width);
    void SetMouseScale(float f_width, float f_height);
    void SetAlpha(float f_alpha);
    bool IsVisible() const;

    bool Poll(vr::VREvent_t &f_event);
    void Update();
};
