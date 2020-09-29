#include "stdafx.h"

#include "Utils/WindowCapturer.h"

#include "Utils/TexturePooler.h"
#include "Utils/Utils.h"

const SL::Screen_Capture::ImageBGRA g_fillColor = { 0U, 0U, 0U, 255U };

sf::Shader *WindowCapturer::ms_shader = nullptr;
sf::RenderStates WindowCapturer::ms_renderState;

WindowCapturer::WindowCapturer()
{
    m_interfaces = nullptr;

    m_windowsCount = 0U;
    m_activeWindow = std::numeric_limits<size_t>::max();

    m_texture = nullptr;
    m_sprite = new sf::Sprite();
    m_renderTexture = nullptr;
    m_bufferUpdated = false;

    m_active = false;
    m_stale = false;

    m_lastTick = 0U;
    m_captureDelay = std::chrono::milliseconds(66U);
}
WindowCapturer::~WindowCapturer()
{
    m_active = false;
    delete m_interfaces;
    if(m_texture) TexturePooler::DeleteTexture(m_texture);
    delete m_sprite;
}

bool WindowCapturer::StartCapture(size_t f_window)
{
    if(!m_active)
    {
        if(f_window < m_windowsCount)
        {
            m_activeWindow = f_window;

            if(m_texture) TexturePooler::DeleteTexture(m_texture);
            m_texture = TexturePooler::CreateTexture(m_windows[m_activeWindow].Size.x, m_windows[m_activeWindow].Size.y);
            m_sprite->setTexture(*m_texture, true);

            if(m_renderTexture) TexturePooler::DeleteRenderTexture(m_renderTexture);
            m_renderTexture = TexturePooler::CreateRenderTexture(m_windows[m_activeWindow].Size.x, m_windows[m_activeWindow].Size.y);

            m_buffer.assign(static_cast<size_t>(m_windows[m_activeWindow].Size.x*m_windows[m_activeWindow].Size.y), g_fillColor);
            m_buffer.shrink_to_fit();

            m_interfaces = new CaptureInterfaces();
            m_interfaces->m_captureConfiguration = SL::Screen_Capture::CreateCaptureConfiguration(static_cast<SL::Screen_Capture::WindowCallback>(std::bind(&WindowCapturer::GetCapturedWindow, this)));
            m_interfaces->m_captureConfiguration->onNewFrame(std::bind(&WindowCapturer::ProcessCapture, this, std::placeholders::_1, std::placeholders::_2));
            m_interfaces->m_captureInterface = m_interfaces->m_captureConfiguration->start_capturing();
            m_interfaces->m_captureInterface->setFrameChangeInterval(m_captureDelay);

            m_lastTick = GetTickCount64();
            m_stale = false;
            m_bufferUpdated = true; // Enforce update to prevent old data visibilty from VRAM
            m_active = true;
        }
    }
    return m_active;
}
void WindowCapturer::StopCapture()
{
    if(m_active)
    {
        m_stale = false;
        m_bufferUpdated = false;
        m_active = false;

        m_interfaces->m_captureInterface->pause();
        delete m_interfaces;
        m_interfaces = nullptr;
    }
}
void WindowCapturer::Update()
{
    if(m_active)
    {
        if(m_bufferLock.try_lock())
        {
            m_stale = ((GetTickCount64() - m_lastTick) > 5000U);

            if(!m_stale && m_bufferUpdated)
            {
                m_texture->update(reinterpret_cast<unsigned char*>(m_buffer.data()), static_cast<unsigned int>(m_windows[m_activeWindow].Size.x), static_cast<unsigned int>(m_windows[m_activeWindow].Size.y), 0U, 0U);
                if(m_renderTexture->setActive(true))
                {
                    m_renderTexture->clear();
                    m_renderTexture->draw(*m_sprite, ms_renderState);
                    m_renderTexture->display();
                    m_renderTexture->setActive(false);
                }
                m_bufferUpdated = false;
            }
            m_bufferLock.unlock();
        }
    }
}

void WindowCapturer::SetDelay(size_t f_delay)
{
    m_captureDelay = std::chrono::milliseconds(f_delay);
    if(m_active)
    {
        m_interfaces->m_captureInterface->pause();
        m_interfaces->m_captureInterface->setFrameChangeInterval(m_captureDelay);
        m_interfaces->m_captureInterface->resume();
    }
}

void* WindowCapturer::GetTextureHandle() const
{
    void *l_result = nullptr;
    if(m_texture) l_result = reinterpret_cast<void*>(static_cast<uintptr_t>(m_renderTexture->getTexture().getNativeHandle()));
    return l_result;
}

void WindowCapturer::UpdateWindows()
{
    if(!m_active)
    {
        m_windows = SL::Screen_Capture::GetWindows();
        for(auto l_iter = m_windows.begin(); l_iter != m_windows.end();)
        {
#ifdef _WIN32
            if((strlen(l_iter->Name) < 1U) || (l_iter->Size.x <= 10) || (l_iter->Size.y <= 10) || !IsWindowVisible(reinterpret_cast<HWND>(l_iter->Handle))) l_iter = m_windows.erase(l_iter);
#elif __linux__
            if((strlen(l_iter->Name) < 1U) || (l_iter->Size.x <= 10) || (l_iter->Size.y <= 10)) l_iter = m_windows.erase(l_iter);
#endif
            else ++l_iter;
        }
        m_windowsCount = m_windows.size();
    }
}
const SL::Screen_Capture::Window* WindowCapturer::GetWindowInfo(size_t f_window) const
{
    SL::Screen_Capture::Window *l_result = nullptr;
    if(f_window < m_windowsCount) l_result = const_cast<SL::Screen_Capture::Window*>(&m_windows[f_window]);
    return l_result;
}
size_t WindowCapturer::GetWindowsCount() const
{
    return m_windowsCount;
}

std::vector<SL::Screen_Capture::Window> WindowCapturer::GetCapturedWindow()
{
    std::vector<SL::Screen_Capture::Window> l_windows;
    if(m_activeWindow != std::numeric_limits<size_t>::max()) l_windows.push_back(m_windows[m_activeWindow]);
    return l_windows;
}
void WindowCapturer::ProcessCapture(const SL::Screen_Capture::Image &f_img, const SL::Screen_Capture::Window &f_window)
{
    if(m_active)
    {
        m_bufferLock.lock();
        ExtractScreenCaptureImage(f_img, reinterpret_cast<unsigned char*>(m_buffer.data()), m_buffer.size()*sizeof(SL::Screen_Capture::ImageBGRA));
        m_lastTick = GetTickCount64();
        m_bufferUpdated = true;
        m_bufferLock.unlock();
    }
}

// Static
void WindowCapturer::InitStaticResources()
{
    if(!ms_shader)
    {
        ms_shader = new sf::Shader();
        if(ms_shader->loadFromFile("shaders/frag_color_swap.glsl", sf::Shader::Fragment))
        {
            ms_shader->setUniform("gTexture0", sf::Shader::CurrentTexture);
            ms_renderState = sf::RenderStates(ms_shader);
        }
    }
}

void WindowCapturer::RemoveStaticResources()
{
    if(ms_shader)
    {
        delete ms_shader;
        ms_shader = nullptr;
        ms_renderState = sf::RenderStates();
    }
}
