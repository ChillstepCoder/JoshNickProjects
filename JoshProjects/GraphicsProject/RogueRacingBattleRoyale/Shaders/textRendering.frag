//textRendering.frag

#version 130
in vec2 fragmentUV;
in vec4 fragmentColor;
out vec4 color;
uniform sampler2D mySampler;
void main() {
    vec2 flippedUV = vec2(fragmentUV.x, fragmentUV.y);
    float alpha = texture(mySampler, flippedUV).r;
    color = vec4(fragmentColor.rgb, alpha);
}