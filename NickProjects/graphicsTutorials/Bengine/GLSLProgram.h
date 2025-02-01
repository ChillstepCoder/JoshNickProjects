#pragma once

#include <string>
#include <GL/glew.h>

namespace Bengine {

    class GLSLProgram
    {
    public:
        GLSLProgram();
        ~GLSLProgram();

        void compileShaders(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);

        void linkShaders();

        void addAttribute(const std::string& attributeName);

        GLint getUniformLocation(const std::string& uniformName);

        void cleanup() {
            if (m_programID != 0) {
                glDeleteProgram(m_programID);
                m_programID = 0;
            }
        }

        void use();
        void unuse();
    private:

        int m_numAttributes;

        void compileShader(const std::string& filePath, GLuint id);

        GLuint m_programID;

        GLuint m_vertexShaderID;
        GLuint m_fragmentShaderID;
    };

}