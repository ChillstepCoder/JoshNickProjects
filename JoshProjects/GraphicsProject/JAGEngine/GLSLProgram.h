//GLSLProgram.h

#pragma once
#include <string>
#include <GL/glew.h>

namespace JAGEngine {
  class GLSLProgram
  {
  public:
    GLSLProgram();
    ~GLSLProgram();
    void compileShaders(const std::string vertexShaderFilePath, const std::string fragmentShaderFilepath);

    void linkShaders();

    void addAttribute(const std::string& attributeName);

    GLint getUniformLocation(const std::string& uniformName);

    void use();
    void unuse();

    //Getters
    GLuint getProgramID() const { return _programID; }

  private:
    int _numAttributes;

    void compileShader(const std::string& filePath, GLuint id);

    GLuint _programID;

    GLuint _vertexShaderID;
    GLuint _fragmentShaderID;
  };
}
