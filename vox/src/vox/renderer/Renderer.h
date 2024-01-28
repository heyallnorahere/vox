#pragma once

namespace vox {
    class Renderer {
    public:
        enum class API { Vulkan };
        struct Info {
            API RendererAPI;
            uint32_t Major, Minor, Patch;
        };

        static std::unique_ptr<Renderer> Create(API api);

        virtual ~Renderer() = default;

        virtual void Query(Info& info) = 0;
    };
}