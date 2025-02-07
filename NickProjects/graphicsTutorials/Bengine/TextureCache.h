#pragma once
#include <map>
#include "GLTexture.h"
#include <string>

namespace Bengine {

    class TextureCache
    {
    public:
        TextureCache();
        ~TextureCache();

        GLTexture getTexture(std::string texturePath, Bengine::TextureFilterMode filterMode);

    private:
        std::map<std::string, GLTexture> m_textureMap;
    };

}