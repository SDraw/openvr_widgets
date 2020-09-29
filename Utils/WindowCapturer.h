#pragma once

class WindowCapturer final
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
    sf::Sprite *m_sprite;
    sf::RenderTexture *m_renderTexture;
    static sf::Shader *ms_shader;
    static sf::RenderStates ms_renderState;
    std::vector<SL::Screen_Capture::ImageBGRA> m_buffer;
    std::mutex m_bufferLock;
    bool m_bufferUpdated;

    std::atomic<bool> m_active;
    bool m_stale;

    std::chrono::milliseconds m_captureDelay;
    unsigned long long m_lastTick;

    WindowCapturer(const WindowCapturer &that) = delete;
    WindowCapturer& operator=(const WindowCapturer &that) = delete;

    std::vector<SL::Screen_Capture::Window> GetCapturedWindow();
    void ProcessCapture(const SL::Screen_Capture::Image &f_img, const SL::Screen_Capture::Window &f_window);
public:
    WindowCapturer();
    ~WindowCapturer();

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

    static void InitStaticResources();
    static void RemoveStaticResources();

};
