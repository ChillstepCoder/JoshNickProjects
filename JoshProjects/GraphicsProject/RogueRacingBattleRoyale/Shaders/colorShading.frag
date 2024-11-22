// colorShading.frag
#version 130

in vec2 fragmentUV;
in vec4 fragmentColor;
uniform sampler2D textureSampler;

out vec4 color;

void main() {
    vec4 textureColor = texture(textureSampler, fragmentUV);
    color = textureColor * fragmentColor;
}