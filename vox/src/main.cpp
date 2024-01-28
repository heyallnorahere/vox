#include "voxpch.h"

#include "vox/core/Application.h"

namespace vox {
    int Entrypoint(const std::vector<std::string>& args) {
        tracy::SetThreadName("Vox");
        ZoneScopedC(tracy::Color::Grey20);

        vox::Application::Create(args);
        int retval = vox::Application::Get().Run();

        vox::Application::Destroy();
        return retval;
    }
} // namespace vox

int main(int argc, const char** argv) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }

    return vox::Entrypoint(args);
}