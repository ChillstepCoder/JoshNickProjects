#pragma once

#include <windows.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <sstream>
#include <psapi.h>
#include <algorithm>

namespace Bengine {

    class DebugOpenGL
    {
    public:
        static bool IsRunningUnderNsight();
        static void APIENTRY glDebugOutput(GLenum source,
            GLenum type,
            unsigned int id,
            GLenum severity,
            GLsizei length,
            const char* message,
            const void* userParam);

        static int GetErrorCount() { return s_errorCount; }
        static void ResetErrorCount() { s_errorCount = 0; }
    private:
        static int s_errorCount;
    };

}