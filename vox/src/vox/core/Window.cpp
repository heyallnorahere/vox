#include "voxpch.h"
#include "vox/core/Window.h"

#ifdef VOX_PLATFORM_desktop
#include "vox/platform/desktop/DesktopWindow.h"
#endif

namespace vox {
    Ref<Window> Window::Create(const std::string& title, uint32_t width, uint32_t height) {
        ZoneScoped;
        Ref<Window> result;

#ifdef VOX_PLATFORM_desktop
        result = new DesktopWindow(title, width, height);
#endif

        return result;
    }

    void Window::SendResizeEvent(uint32_t width, uint32_t height) {
        ZoneScoped;

        // todo: send event to application
    }
} // namespace vox