#pragma once

#include "vox/core/Ref.h"
#include "vox/renderer/CommandQueue.h"

namespace vox {
    class GraphicsDevice : public RefCounted {
    public:
        virtual ~GraphicsDevice() = default;

        virtual bool HasQueue(GraphicsQueueType::Flags flags) = 0;
        virtual CommandQueue& GetQueue(GraphicsQueueType::Flags flags) = 0;
    };
} // namespace vox