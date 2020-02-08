#pragma once

class WindowGrabber final
{
    struct CaptureInterfaces
    {
        std::shared_ptr<SL::Screen_Capture::ICaptureConfiguration<SL::Screen_Capture::WindowCaptureCallback>> m_captureConfiguration;
        std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> m_captureInterface;
    };
    CaptureInterfaces *m_interfaces;

    std::vector<SL::Screen_Capture::Window> m_windows;
    size_t m_windowsCount;
    size_t m_activeWindow;

    sf::Texture *m_texture;
    std::vector<SL::Screen_Capture::ImageBGRA> m_buffer;
    std::mutex m_bufferLock;

    std::atomic<bool> m_active;
    bool m_stale;

    std::chrono::milliseconds m_captureDelay;
    ULONGLONG m_lastTick;

    WindowGrabber(const WindowGrabber &that) = delete;
    WindowGrabber& operator=(const WindowGrabber &that) = delete;
public:
    WindowGrabber();
    ~WindowGrabber();

    bool StartCapture(size_t f_window);
    void StopCapture();
    void Update();

    inline bool IsActive() const { return m_active; }
    inline bool IsStale() const { return m_stale; }

    void SetDelay(size_t f_delay);

    size_t GetWindowsCount() const;
    const SL::Screen_Capture::Window* GetWindowInfo(size_t f_window) const;
    void UpdateWindows();

    void* GetTextureHandle() const;

    // Internal use only
    std::vector<SL::Screen_Capture::Window> GetCapturedWindow();
    void ProcessCapture(const SL::Screen_Capture::Image &f_img, const SL::Screen_Capture::Window &f_window);
};
