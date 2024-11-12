// textureShading.frag
#version 130
in vec2 fragmentPosition;
in vec4 fragmentColor;
in vec2 fragmentUV;
out vec4 color;
uniform sampler2D mySampler;

void main() {
    // If we're rendering with texture ID 0 (our untextured case),
    // just use the fragment color directly
    vec4 textureColor = texture(mySampler, fragmentUV);
    if (textureColor.a == 0.0) {
        // No texture or empty texture, just use color directly
        color = fragmentColor;
    } else {
        // Has texture, multiply color with texture
        color = fragmentColor * textureColor;
    }
}