#include "stdafx.h"

#include "Core/WindowGrabber.h"
#include "Core/Config.h"
#include "Utils/Utils.h"

const SL::Screen_Capture::ImageBGRA g_FillColor = { 0U, 0U, 0U, 255U };

WindowGrabber::WindowGrabber()
{
    m_texture = nullptr;
    m_active = false;
    m_interfaces = nullptr;
    m_windowsCount = 0U;
    m_activeWindow = std::numeric_limits<size_t>::max();
}
WindowGrabber::~WindowGrabber()
{
    m_active = false;
    delete m_interfaces;
    delete m_texture;
}

bool WindowGrabber::StartCapture(size_t f_window)
{
    if(!m_active)
    {
        if(f_window < m_windowsCount)
        {
            m_activeWindow = f_window;

            // Create new texture with different OpenGL handle to prevent texture locking in vrcompositor
            sf::Texture *l_texture = new sf::Texture();
            if(l_texture->create(m_windows[m_activeWindow].Size.x, m_windows[m_activeWindow].Size.y))
            {
                delete m_texture;
                m_texture = l_texture;

                m_buffer.assign(m_windows[m_activeWindow].Size.x*m_windows[m_activeWindow].Size.y, g_FillColor);
                m_buffer.shrink_to_fit();

                m_interfaces = new CaptureInterfaces();
                SL::Screen_Capture::WindowCallback l_windowCallback([this]()
                {
                    return this->GetCapturedWindow();
                });
                m_interfaces->m_captureConfiguration = SL::Screen_Capture::CreateCaptureConfiguration(l_windowCallback);
                SL::Screen_Capture::WindowCaptureCallback l_captureCallback([this](const SL::Screen_Capture::Image &f_img, const SL::Screen_Capture::Window &f_window)
                {
                    this->ProcessCapture(f_img, f_window);
                });
                m_interfaces->m_captureConfiguration->onNewFrame(l_captureCallback);
                m_interfaces->m_captureInterface = m_interfaces->m_captureConfiguration->start_capturing();
                m_interfaces->m_captureInterface->setFrameChangeInterval(std::chrono::milliseconds(Config::GetCaptureRate()));

                m_active = true;
            }
            else delete l_texture;
        }
    }
    return m_active;
}
void WindowGrabber::StopCapture()
{
    if(m_active)
    {
        m_active = false;

        delete m_interfaces;
        m_interfaces = nullptr;
    }
}
void WindowGrabber::Update()
{
    if(m_active)
    {
        if(m_bufferLock.try_lock())
        {
            m_texture->update(reinterpret_cast<unsigned char*>(m_buffer.data()), static_cast<unsigned int>(m_windows[m_activeWindow].Size.x), static_cast<unsigned int>(m_windows[m_activeWindow].Size.y), 0U, 0U);
            m_bufferLock.unlock();
        }
    }
}

void* WindowGrabber::GetTextureHandle() const
{
    void *l_result = nullptr;
    if(m_texture) l_result = reinterpret_cast<void*>(static_cast<uintptr_t>(m_texture->getNativeHandle()));
    return l_result;
}

void WindowGrabber::UpdateWindows()
{
    if(!m_active)
    {
        m_windows = SL::Screen_Capture::GetWindows();
        for(auto l_iter = m_windows.begin(); l_iter != m_windows.end();)
        {
            if((strlen(l_iter->Name) < 1U) || (l_iter->Size.x <= 10) || (l_iter->Size.y <= 10) || !IsWindowVisible(reinterpret_cast<HWND>(l_iter->Handle))) l_iter = m_windows.erase(l_iter);
            else ++l_iter;
        }
        m_windowsCount = m_windows.size();
    }
}
const SL::Screen_Capture::Window* WindowGrabber::GetWindowInfo(size_t f_window) const
{
    SL::Screen_Capture::Window *l_result = nullptr;
    if(f_window < m_windowsCount) l_result = const_cast<SL::Screen_Capture::Window*>(&m_windows[f_window]);
    return l_result;
}
size_t WindowGrabber::GetWindowsCount() const
{
    return m_windowsCount;
}

std::vector<SL::Screen_Capture::Window> WindowGrabber::GetCapturedWindow()
{
    std::vector<SL::Screen_Capture::Window> l_windows;
    if(m_activeWindow != std::numeric_limits<size_t>::max()) l_windows.push_back(m_windows[m_activeWindow]);
    return l_windows;
}
void WindowGrabber::ProcessCapture(const SL::Screen_Capture::Image &f_img, const SL::Screen_Capture::Window &f_window)
{
    if(m_active)
    {
        m_bufferLock.lock();
        ExtractAndConvertToRGBA(f_img, reinterpret_cast<unsigned char*>(m_buffer.data()), m_buffer.size()*sizeof(SL::Screen_Capture::ImageBGRA));
        m_bufferLock.unlock();
    }
}
