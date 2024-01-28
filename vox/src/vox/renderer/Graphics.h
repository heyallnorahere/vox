#pragma once

#include "Renderer.h"

namespace vox {
    class Graphics {
    public:
        static void Init(Renderer::API api);
        static void Shutdown();

        Graphics() = delete;

        static Renderer& GetRenderer();
    };
} // namespace vox