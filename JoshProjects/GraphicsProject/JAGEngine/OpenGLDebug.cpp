// OpenGLDebug.cpp

#include "OpenGLDebug.h"
#include <iostream>



namespace JAGEngine {
  int OpenGLDebug::s_errorCount = 0;

  bool OpenGLDebug::IsRunningUnderNsight() {
    HMODULE hMods[1024];
    DWORD cbNeeded;
    unsigned int i;

    static int cachedValue = 0;
    if (cachedValue == 1) return true;
    if (cachedValue == 2) return false;

    HANDLE hProcess = GetCurrentProcess();

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
      for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
        TCHAR szModName[MAX_PATH];
        if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
#ifdef UNICODE
          std::wstring wModuleName = szModName;
          std::string moduleName(wModuleName.begin(), wModuleName.end());
#else
          std::string moduleName = szModName;
#endif

          std::transform(moduleName.begin(), moduleName.end(), moduleName.begin(), ::tolower);
          if (moduleName.find("injection") != std::string::npos) {
            cachedValue = 1;
            return true;
          }
        }
      }
    }
    cachedValue = 2;
    return false;
  }

  void APIENTRY OpenGLDebug::glDebugOutput(GLenum source, GLenum type, unsigned int id,
    GLenum severity, GLsizei length, const char* message, const void* userParam) {
    // ignore non-significant error/warning codes
    // 131218 - performance - recompiling shader... hmmm
    if (id == 131169/*driver allocated storage**/ || id == 131185 || id == 131218 || id == 131204 || id == 131186/*Buffer Performance warning*/) return;

    std::stringstream ss;

    ss << "---------------" << std::endl;
    ss << "Debug message (" << id << " - " << std::hex << "0x" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             ss << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   ss << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: ss << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     ss << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     ss << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           ss << "Source: Other"; break;
    } ss << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               ss << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ss << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  ss << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         ss << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         ss << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              ss << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          ss << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           ss << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               ss << "Type: Other"; break;
    } ss << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         ss << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       ss << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          ss << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: ss << "Severity: notification"; break;
    } ss << std::endl;
    ss << std::endl;
    std::cerr << ss.str();

    if (type == GL_DEBUG_TYPE_ERROR) {
      s_errorCount++;
    }

    // Debug break crashes NSight
    if (!IsRunningUnderNsight()) {
      __debugbreak();
    }
  }

}