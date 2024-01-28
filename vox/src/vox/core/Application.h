#pragma once
#include "vox/core/Window.h"

namespace vox {
    class Application {
    public:
        static bool Create(const std::vector<std::string>& args);
        static void Destroy();
        static Application& Get();

        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        int Run();

        const Ref<Window>& GetWindow() { return m_Window; }

    private:
        Application();

        void Initialize(const std::vector<std::string>& args);

        Ref<Window> m_Window;
        bool m_Running;
    };
} // namespace vox