#include "voxpch.h"
#include "vox/platform/desktop/DesktopWindow.h"

#include "vox/platform/vulkan/VulkanBase.h"
#include <GLFW/glfw3.h>

namespace vox {
    static std::atomic<uint32_t> s_WindowCount;

    static std::string GetGLFWError() {
        ZoneScoped;

        const char* errorString;
        int error = glfwGetError(&errorString);

        return "(" + std::to_string(error) + ")" + std::string(errorString);
    }

    DesktopWindow::DesktopWindow(const std::string& title, uint32_t width, uint32_t height) {
        ZoneScoped;
        spdlog::info("Creating GLFW window: {}, {}x{}", title, width, height);

        if (s_WindowCount++ == 0) {
            bool initialized = glfwInit();
            if (!initialized) {
                spdlog::error(GetGLFWError());
                throw std::runtime_error("Failed to initialize GLFW!");
            }
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_Window = glfwCreateWindow((int)width, (int)height, title.c_str(), nullptr, nullptr);
        if (m_Window == nullptr) {
            spdlog::error(GetGLFWError());
            throw std::runtime_error("Failed to create GLFW window!");
        }

        glfwSetWindowUserPointer(m_Window, this);
        glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            auto _this = (DesktopWindow*)glfwGetWindowUserPointer(window);
            _this->SendResizeEvent((uint32_t)width, (uint32_t)height);
        });
    }

    DesktopWindow::~DesktopWindow() {
        ZoneScoped;
        glfwDestroyWindow(m_Window);

        if (--s_WindowCount == 0) {
            glfwTerminate();
        }
    }

    void DesktopWindow::Update() {
        ZoneScoped;
        glfwPollEvents();
    }

    void DesktopWindow::GetRequiredVulkanExtensions(std::vector<std::string>& extensions) const {
        extensions.clear();
        if (!glfwVulkanSupported()) {
            return;
        }

        uint32_t count;
        const char** names = glfwGetRequiredInstanceExtensions(&count);

        for (uint32_t i = 0; i < count; i++) {
            extensions.push_back(names[i]);
        }
    }

    void* DesktopWindow::CreateVulkanSurface(void* instance) const {
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface((VkInstance)instance, m_Window, nullptr, &surface) !=
            VK_SUCCESS) {
            surface = nullptr;
        }

        return surface;
    }

    bool DesktopWindow::ShouldClose() const {
        ZoneScoped;
        return glfwWindowShouldClose(m_Window);
    }

    uint32_t DesktopWindow::GetWidth() const {
        ZoneScoped;

        int width;
        glfwGetFramebufferSize(m_Window, &width, nullptr);

        return (uint32_t)width;
    }

    uint32_t DesktopWindow::GetHeight() const {
        ZoneScoped;

        int height;
        glfwGetFramebufferSize(m_Window, nullptr, &height);

        return (uint32_t)height;
    }
} // namespace vox