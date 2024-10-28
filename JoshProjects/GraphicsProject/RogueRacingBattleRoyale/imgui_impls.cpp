// imgui_impls.cpp

// Tell ImGui and implementations that this is the implementation file
#define IMGUI_IMPLEMENTATION
#define IMGUI_IMPL_OPENGL3_IMPLEMENTATION
#define IMGUI_IMPL_SDL2_IMPLEMENTATION

// Include ImGui core
#include "ImGui/imgui.cpp"
#include "ImGui/imgui_draw.cpp"
#include "ImGui/imgui_widgets.cpp"
#include "ImGui/imgui_tables.cpp"

// Include ImGui implementations
#include "ImGui/imgui_impl_opengl3.cpp"
#include "ImGui/imgui_impl_sdl2.cpp"
