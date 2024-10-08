//textRendering.frag

#version 130
in vec2 fragmentUV;
in vec4 fragmentColor;
out vec4 color;
uniform sampler2D mySampler;
void main() {
    float alpha = texture(mySampler, fragmentUV).r;
    color = vec4(fragmentColor.rgb, alpha);
}