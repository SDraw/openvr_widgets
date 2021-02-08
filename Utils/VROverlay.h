#pragma once

class Transformation;
class VROverlay final
{
    vr::VROverlayHandle_t m_handle;
    vr::Texture_t m_texture;

    Transformation *m_transform;
public:
    VROverlay();
    ~VROverlay();

    bool Create(const std::string &f_key, const std::string &f_name);
    void Destroy();
    bool IsValid() const;

    void SetTexture(unsigned int f_name);
    void SetTexture(const std::string &f_path);
    void SetTexture(unsigned int f_width, unsigned int f_height);
    void SetInputMethod(vr::VROverlayInputMethod f_method);
    void SetFlag(vr::VROverlayFlags f_flag, bool f_state);
    void SetWidth(float f_width);
    void SetMouseScale(float f_width, float f_height);
    void SetAlpha(float f_alpha);
    void SetColor(float f_red, float f_green, float f_blue);
    void SetOrder(unsigned int f_order);
    void SetIntersection(float l_width, float f_height);
    void Show();
    void Hide();
    bool IsVisible() const;

    Transformation* GetTransform() const;

    bool Intersect(const glm::vec3 &f_pos, const glm::vec3 &f_dir, glm::vec2 &f_uv);
    bool Poll(vr::VREvent_t &f_event);
    void Update(const VROverlay *f_parent = nullptr);
};
