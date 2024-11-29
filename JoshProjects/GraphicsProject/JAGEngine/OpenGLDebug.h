// OpenGLDebug.h

#pragma once
#include <windows.h>
#include <sstream>
#include <string>
#include <algorithm>
#include <GL/glew.h>
#include <Psapi.h>

namespace JAGEngine {

  class OpenGLDebug {
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
