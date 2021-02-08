#include "stdafx.h"

#include "Utils/VROverlay.h"
#include "Utils/Transformation.h"

VROverlay::VROverlay()
{
    m_handle = vr::k_ulOverlayHandleInvalid;
    m_texture.eType = vr::TextureType_OpenGL;
    m_texture.eColorSpace = vr::ColorSpace_Gamma;
    m_texture.handle = nullptr;

    m_transform = new Transformation();
}

VROverlay::~VROverlay()
{
    delete m_transform;
}

bool VROverlay::Create(const std::string &f_key, const std::string &f_name)
{
    if(m_handle == vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->CreateOverlay(f_key.c_str(), f_name.c_str(), &m_handle);
    }
    return (m_handle != vr::k_ulOverlayHandleInvalid);
}

void VROverlay::Destroy()
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->ClearOverlayTexture(m_handle);
        vr::VROverlay()->DestroyOverlay(m_handle);
        m_handle = vr::k_ulOverlayHandleInvalid;
        m_texture.handle = nullptr;
    }
}

bool VROverlay::IsValid() const
{
    return (m_handle != vr::k_ulOverlayHandleInvalid);
}

void VROverlay::SetTexture(unsigned int f_name)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        m_texture.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(f_name));
    }
}

void VROverlay::SetTexture(const std::string &f_path)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->SetOverlayFromFile(m_handle, f_path.c_str());
    }
}

void VROverlay::SetTexture(unsigned int f_width, unsigned int f_height)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        std::vector<unsigned char> l_pixels(f_width*f_height*4U, 255U);
        auto test = vr::VROverlay()->SetOverlayRaw(m_handle, l_pixels.data(), f_width, f_height, 4U);
        void *a = nullptr;
    }
}

void VROverlay::SetInputMethod(vr::VROverlayInputMethod f_method)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->SetOverlayInputMethod(m_handle, f_method);
    }
}

void VROverlay::SetFlag(vr::VROverlayFlags f_flag, bool f_state)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->SetOverlayFlag(m_handle, f_flag, f_state);
    }
}

void VROverlay::SetWidth(float f_width)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->SetOverlayWidthInMeters(m_handle, f_width);
    }
}

void VROverlay::SetMouseScale(float f_width, float f_height)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        const vr::HmdVector2_t l_scale = { f_width, f_height };
        vr::VROverlay()->SetOverlayMouseScale(m_handle, &l_scale);
    }
}

void VROverlay::SetAlpha(float f_alpha)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->SetOverlayAlpha(m_handle, f_alpha);
    }
}

void VROverlay::SetColor(float f_red, float f_green, float f_blue)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->SetOverlayColor(m_handle, f_red, f_green, f_blue);
    }
}

void VROverlay::SetOrder(unsigned int f_order)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->SetOverlaySortOrder(m_handle, f_order);
    }
}

void VROverlay::SetIntersection(float l_width, float f_height)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlayIntersectionMaskPrimitive_t l_mask;
        l_mask.m_nPrimitiveType = vr::OverlayIntersectionPrimitiveType_Rectangle;
        l_mask.m_Primitive.m_Rectangle.m_flTopLeftX = 0.f;
        l_mask.m_Primitive.m_Rectangle.m_flTopLeftY = 0.f;
        l_mask.m_Primitive.m_Rectangle.m_flWidth = l_width;
        l_mask.m_Primitive.m_Rectangle.m_flHeight = f_height;

        vr::VROverlay()->SetOverlayIntersectionMask(m_handle, &l_mask, 1U);
    }
}

void VROverlay::Show()
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->ShowOverlay(m_handle);
    }
}

void VROverlay::Hide()
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->HideOverlay(m_handle);
    }
}

bool VROverlay::IsVisible() const
{
    bool l_result = false;
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        l_result = vr::VROverlay()->IsOverlayVisible(m_handle);
    }
    return l_result;
}

Transformation* VROverlay::GetTransform() const
{
    return m_transform;
}

bool VROverlay::Intersect(const glm::vec3 &f_pos, const glm::vec3 &f_dir, glm::vec2 &f_uv)
{
    bool l_result = false;
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        const vr::VROverlayIntersectionParams_t l_params = { { f_pos.x, f_pos.y, f_pos.z }, { f_dir.x, f_dir.y, f_dir.z }, vr::TrackingUniverseRawAndUncalibrated };
        vr::VROverlayIntersectionResults_t l_intersection;
        if(vr::VROverlay()->ComputeOverlayIntersection(m_handle, &l_params, &l_intersection))
        {
            f_uv.x = l_intersection.vUVs.v[0U];
            f_uv.y = l_intersection.vUVs.v[1U];
            l_result = true;
        }
    }
    return l_result;
}

bool VROverlay::Poll(vr::VREvent_t &f_event)
{
    bool l_result = false;
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        l_result = vr::VROverlay()->PollNextOverlayEvent(m_handle, &f_event, sizeof(vr::VREvent_t));
    }
    return l_result;
}

void VROverlay::Update(const VROverlay *f_parent)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        m_transform->Update(f_parent ? f_parent->GetTransform() : nullptr);
        vr::VROverlay()->SetOverlayTransformAbsolute(m_handle, vr::TrackingUniverseRawAndUncalibrated, &m_transform->GetMatrixVR());

        if(m_texture.handle != nullptr) vr::VROverlay()->SetOverlayTexture(m_handle, &m_texture);
    }
}
