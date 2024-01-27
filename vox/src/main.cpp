#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>
#include <thread>

int main(int argc, const char** argv) {
    ZoneScoped;
    spdlog::info("Hello");

    while (true) {
        ZoneScopedN("Loop");
        std::this_thread::sleep_for(std::chrono::seconds(1));

        spdlog::info("Frame");
        FrameMark;
    }

    return 0;
}