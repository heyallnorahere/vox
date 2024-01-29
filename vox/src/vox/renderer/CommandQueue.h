#pragma once

#include "vox/core/Ref.h"

// api directly taken from code-playground lmao
// dont tell anyone. shhh
namespace vox {
    namespace GraphicsQueueType {
        using Flags = uint8_t;
        enum QueueType : Flags { Graphics = 0x1, Transfer = 0x2, Compute = 0x4, Present = 0x8 };
    }

    class CommandList {
    public:
        virtual bool IsRecording() const = 0;
        virtual GraphicsQueueType::Flags GetUsage() const = 0;
        virtual void* Raw() const = 0;

        virtual void Begin() = 0;
        virtual void End() = 0;
    };

    class CommandQueue {
    public:
        virtual ~CommandQueue() = 0;

        virtual GraphicsQueueType::Flags GetUsage() = 0;

        virtual CommandList& Release() = 0;
        virtual void Submit(CommandList& commandList, bool wait = false) = 0;

        virtual void Wait() = 0;
        virtual void ResetCache() = 0;
    };
} // namespace vox