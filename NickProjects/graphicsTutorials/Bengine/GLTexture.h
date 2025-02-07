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

}