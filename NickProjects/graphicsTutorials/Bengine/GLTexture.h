#pragma once
#include <GL/glew.h>

namespace Bengine {

    enum class TextureFilterMode {
        Nearest,
        Linear,
        COUNT
    };

    struct GLTexture {
        GLuint id;
        int width;
        int height;
    };

    inline void setTextureFilterMode(GLuint Id, TextureFilterMode filter) {

        glBindTexture(GL_TEXTURE_2D, Id);

        switch (filter) {
        case TextureFilterMode::Nearest:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

            break;
        case TextureFilterMode::Linear:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            break;
        default:
            break;

        }
    }

}