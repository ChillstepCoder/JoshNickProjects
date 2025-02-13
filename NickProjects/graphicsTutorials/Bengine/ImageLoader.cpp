#include "ImageLoader.h"
#include "picoPNG.h"
#include "IOManager.h"
#include  "BengineErrors.h"

namespace Bengine {

    GLTexture ImageLoader::loadPNG(std::string filePath, TextureFilterMode filterMode) {
        GLTexture texture = {};

        std::vector<unsigned char> in;
        std::vector<unsigned char> out;

        unsigned long width, height;

        if (IOManager::readFileToBuffer(filePath, in) == false) {
            fatalError("Failed to load PNG file to buffer!");
        }

        int errorCode = decodePNG(out, width, height, &(in[0]), in.size());
        if (errorCode != 0) {
            fatalError("decodePNG failed with error: " + std::to_string(errorCode));
        }

        glGenTextures(1, &(texture.id));

        glBindTexture(GL_TEXTURE_2D, texture.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(out[0]));

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        setTextureFilterMode(texture.id, filterMode); 

        static_assert((int)TextureFilterMode::COUNT == 2, "Update the switch with new filter mode");
        GLfloat value, max_anisotropy = 8.0f; /* don't exceed this value...*/
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &value);

        value = (value > max_anisotropy) ? max_anisotropy : value;
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);

        //Generate the mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        //Unbind the texture
        glBindTexture(GL_TEXTURE_2D, 0);

        texture.width = width;
        texture.height = height;

        //Return a copy of the texture data
        return texture;
    }

}