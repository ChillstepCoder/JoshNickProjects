// Main.cpp
#include "DialogueApp.h"
#include <iostream>

int main(int argc, char** argv) {
    std::cout << "Main function starting\n";

    // Disable fullscreen before creating app
    if (SDL_SetHint("SDL_VIDEO_FULLSCREEN_DISABLED", "1")) {
        std::cout << "Successfully set SDL fullscreen disabled hint\n";
    }
    else {
        std::cout << "Failed to set SDL fullscreen disabled hint\n";
    }

    // Create and run the application
    DialogueApp app;

    std::cout << "Starting app.run()\n";
    app.run();
    std::cout << "app.run() completed\n";

    return 0;
}