#include "stdafx.h"

#include "Utils/VRDashOverlay.h"

VRDashOverlay::VRDashOverlay()
{
    m_handle = vr::k_ulOverlayHandleInvalid;
    m_handleThumb = vr::k_ulOverlayHandleInvalid;
    m_texture.eType = vr::TextureType_OpenGL;
    m_textureThumb.eType = vr::TextureType_OpenGL;
    m_texture.eColorSpace = vr::ColorSpace_Gamma;
    m_textureThumb.eColorSpace = vr::ColorSpace_Gamma;
    m_texture.handle = nullptr;
    m_textureThumb.handle = nullptr;
}

VRDashOverlay::~VRDashOverlay()
{
}

bool VRDashOverlay::Create(const std::string &f_key, const std::string &f_name)
{
    if(m_handle == vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->CreateDashboardOverlay(f_key.c_str(), f_name.c_str(), &m_handle, &m_handleThumb);
    }
    return (m_handle != vr::k_ulOverlayHandleInvalid);
}

void VRDashOverlay::Destroy()
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->ClearOverlayTexture(m_handle);
        vr::VROverlay()->DestroyOverlay(m_handle);
        m_handle = vr::k_ulOverlayHandleInvalid;
        m_texture.handle = nullptr;
    }
    if(m_handleThumb != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->ClearOverlayTexture(m_handleThumb);
        vr::VROverlay()->DestroyOverlay(m_handleThumb);
        m_handleThumb = vr::k_ulOverlayHandleInvalid;
        m_textureThumb.handle = nullptr;
    }
}

bool VRDashOverlay::IsValid() const
{
    return ((m_handle != vr::k_ulOverlayHandleInvalid) && (m_handleThumb != vr::k_ulOverlayHandleInvalid));
}

void VRDashOverlay::SetTexture(unsigned int f_name)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        m_texture.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(f_name));
    }
}

void VRDashOverlay::SetTexture(const std::string &f_path)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->SetOverlayFromFile(m_handle, f_path.c_str());
    }
}

void VRDashOverlay::SetThumbTexture(unsigned int f_name)
{
    if(m_handleThumb != vr::k_ulOverlayHandleInvalid)
    {
        m_textureThumb.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(f_name));
    }
}

void VRDashOverlay::SetThumbTexture(const std::string &f_path)
{
    if(m_handleThumb != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->SetOverlayFromFile(m_handleThumb, f_path.c_str());
    }
}

void VRDashOverlay::SetInputMethod(vr::VROverlayInputMethod f_method)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->SetOverlayInputMethod(m_handle, f_method);
    }
}

void VRDashOverlay::SetFlag(vr::VROverlayFlags f_flag, bool f_state)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->SetOverlayFlag(m_handle, f_flag, f_state);
    }
}

void VRDashOverlay::SetWidth(float f_width)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->SetOverlayWidthInMeters(m_handle, f_width);
    }
}

void VRDashOverlay::SetMouseScale(float f_width, float f_height)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        const vr::HmdVector2_t l_scale = { f_width, f_height };
        vr::VROverlay()->SetOverlayMouseScale(m_handle, &l_scale);
    }
}

void VRDashOverlay::SetAlpha(float f_alpha)
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->SetOverlayAlpha(m_handle, f_alpha);
    }
}

bool VRDashOverlay::IsVisible() const
{
    bool l_result = false;
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        l_result = vr::VROverlay()->IsOverlayVisible(m_handle);
    }
    return l_result;
}

bool VRDashOverlay::Poll(vr::VREvent_t &f_event)
{
    bool l_result = false;
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        l_result = vr::VROverlay()->PollNextOverlayEvent(m_handle, &f_event, sizeof(vr::VREvent_t));
    }
    return l_result;
}

void VRDashOverlay::Update()
{
    if(m_handle != vr::k_ulOverlayHandleInvalid)
    {
        if(m_texture.handle != nullptr) vr::VROverlay()->SetOverlayTexture(m_handle, &m_texture);
    }
    if(m_handleThumb != vr::k_ulOverlayHandleInvalid)
    {
        if(m_textureThumb.handle != nullptr) vr::VROverlay()->SetOverlayTexture(m_handleThumb, &m_textureThumb);
    }
}
