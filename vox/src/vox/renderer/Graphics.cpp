#include "voxpch.h"
#include "vox/renderer/Graphics.h"

namespace vox {
    struct GraphicsData {
        Renderer::API API;
        std::unique_ptr<Renderer> RendererInstance;
    };

    static std::unique_ptr<GraphicsData> s_Graphics;
    void Graphics::Init(Renderer::API api) {
        ZoneScoped;
        if (s_Graphics) {
            return;
        }

        s_Graphics = std::make_unique<GraphicsData>();
        s_Graphics->API = api;
        s_Graphics->RendererInstance = Renderer::Create(api);
    }

    void Graphics::Shutdown() {
        ZoneScoped;
        if (!s_Graphics) {
            return;
        }

        s_Graphics.reset();
    }

    Renderer& Graphics::GetRenderer() { return *s_Graphics->RendererInstance; }
}