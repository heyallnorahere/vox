#include "voxpch.h"
#include "vox/core/Application.h"

int main(int argc, const char** argv) {
    tracy::SetThreadName("Vox main thread");
    ZoneScoped;

    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }

    vox::Application::Create(args);
    int retval = vox::Application::Get().Run();

    vox::Application::Destroy();
    return retval;
}