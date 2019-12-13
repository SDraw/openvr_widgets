#pragma once


class WindowGrabber final
{
    sf::Texture *m_texture;
    std::vector<SL::Screen_Capture::ImageBGRA> m_buffer;
    std::mutex m_bufferLock;
    std::atomic<bool> m_active;

    struct CaptureInterfaces
    {
        std::shared_ptr<SL::Screen_Capture::ICaptureConfiguration<SL::Screen_Capture::WindowCaptureCallback>> m_captureConfiguration;
        std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> m_captureInterface;
    };
    CaptureInterfaces *m_interfaces;
    std::vector<SL::Screen_Capture::Window> m_windows;
    size_t m_activeWindow;

    WindowGrabber(const WindowGrabber &that) = delete;
    WindowGrabber& operator=(const WindowGrabber &that) = delete;
public:
    WindowGrabber();
    ~WindowGrabber();

    bool StartCapture(size_t f_window);
    void StopCapture();
    void Update();

    inline bool IsActive() const { return m_active; }

    void UpdateWindows();
    const SL::Screen_Capture::Window* GetWindowInfo(size_t f_window) const;
    void* GetTextureHandle() const;
    size_t GetWindowsCount() const;

    // Internal use only
    std::vector<SL::Screen_Capture::Window> GetCapturedWindow();
    void ProcessCapture(const SL::Screen_Capture::Image &f_img, const SL::Screen_Capture::Window &f_window);
};
