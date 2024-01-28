#pragma once

#include "vox/core/Ref.h"

namespace vox {
    namespace GraphicsQueueType {
        enum QueueType : uint8_t { Graphics = 0x1, Transfer = 0x2, Compute = 0x4, Present = 0x8 };
    }

    class GraphicsDevice : public RefCounted {
    public:
        virtual ~GraphicsDevice() = default;

        // todo: interface functions
    };
} // namespace vox