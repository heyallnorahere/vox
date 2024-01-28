#pragma once
#include "vox/core/Ref.h"

namespace vox {
    class Window : public RefCounted {
    public:
        static Ref<Window> Create(const std::string& title, uint32_t width, uint32_t height);

        virtual ~Window() = default;

        virtual void Update() = 0;

        virtual void GetRequiredVulkanExtensions(std::vector<std::string>& extensions) const = 0;
        virtual void* CreateVulkanSurface(void* instance) const = 0;

        virtual bool ShouldClose() const = 0;
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

    protected:
        void SendResizeEvent(uint32_t width, uint32_t height);
    };
} // namespace vox