#pragma once
#include "TextureCache.h"
#include <string>

namespace Bengine {

    class ResourceManager
    {
    public:
        static GLTexture getTexture(std::string texturePath, Bengine::TextureFilterMode filterMode);


    private:
        static TextureCache m_textureCache;
    };

}