#pragma once
#include "vox/core/Window.h"

struct GLFWwindow;
namespace vox {
    class DesktopWindow : public Window {
    public:
        DesktopWindow(const std::string& title, uint32_t width, uint32_t height);
        virtual ~DesktopWindow() override;

        virtual void Update() override;

        virtual bool ShouldClose() const override;
        virtual uint32_t GetWidth() const override;
        virtual uint32_t GetHeight() const override;

    protected:
        GLFWwindow* m_Window;
    };
} // namespace vox