// imgui_impls.cpp

// Implementation defines must come first
#define IMGUI_IMPLEMENTATION
#define IMGUI_IMPL_OPENGL3_IMPLEMENTATION
#define IMGUI_IMPL_SDL2_IMPLEMENTATION

// Then include core ImGui files
#include "ImGui/imgui.cpp"
#include "ImGui/imgui_draw.cpp"
#include "ImGui/imgui_widgets.cpp"
#include "ImGui/imgui_tables.cpp"
#include "ImGui/imgui_demo.cpp"

// Include ImGui stdlib support for std::string input fields
#include "ImGui/imgui_stdlib.cpp"

// Finally include implementations
#include "ImGui/imgui_impl_opengl3.cpp"
#include "ImGui/imgui_impl_sdl2.cpp"