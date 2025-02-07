#include "TextureCache.h"
#include "ImageLoader.h"
#include <iostream>

namespace Bengine {

    TextureCache::TextureCache() {

    }

    TextureCache::~TextureCache() {

    }

    GLTexture TextureCache::getTexture(std::string texturePath, Bengine::TextureFilterMode filterMode) {
        // Note: Filter mode will not apply if texture is already cached!

        //look up the texture and see if its in the map
        auto mit = m_textureMap.find(texturePath);

        //check if its not in the map
        if (mit == m_textureMap.end()) {
            GLTexture newTexture = ImageLoader::loadPNG(texturePath, filterMode);

            //Insert it into the map
            m_textureMap.insert(make_pair(texturePath, newTexture));

            return newTexture;
        }
        return mit->second;

    }

}