#include "vox/core/Application.h"
#include "voxpch.h"

namespace vox {
    static std::unique_ptr<Application> s_Instance;

    bool Application::Create(const std::vector<std::string>& args) {
        ZoneScoped;

        if (s_Instance) {
            return false;
        }

        s_Instance = std::unique_ptr<Application>(new Application);
        s_Instance->Initialize(args);

        return true;
    }

    void Application::Destroy() { s_Instance.reset(); }
    Application& Application::Get() { return *s_Instance; }

    Application::~Application() {
        ZoneScoped;

        // todo: if necessary, explicitly release any resources
    }

    int Application::Run() {
        ZoneScoped;
        if (m_Running) {
            spdlog::warn("Attempted to start application twice!");
            return 0;
        }

        spdlog::info("Starting application...");
        m_Running = true;
        while (!m_Window->ShouldClose()) {
            ZoneScopedN("Game loop");

            std::this_thread::sleep_for(std::chrono::seconds(1));
            // todo: frame

            m_Window->Update();
            FrameMark;
        }

        spdlog::info("Stopping!");
        m_Running = false;
        return 0;
    }

    Application::Application() {
        ZoneScoped;

        m_Running = false;
    }

    void Application::Initialize(const std::vector<std::string>& args) {
        ZoneScoped;
        spdlog::info("Initializing application...");

        // todo: parse arguments
        // maybe select gpu?

        m_Window = Window::Create("vox", 1600, 900);
        spdlog::info("Application initialized!");
    }
} // namespace vox