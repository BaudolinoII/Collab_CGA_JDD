#version 330 core

in vec3 our_uv;
out vec4 color;

uniform samplerCube skybox;
uniform float uppderLimit = 0.02;
uniform float lowerLimit = 0.0;
uniform vec3 fogColor;

void main(){
    color = texture(skybox, our_uv);
    float factor = (our_uv.y - lowerLimit) / (uppderLimit - lowerLimit);
    factor = clamp(factor, 0.0, 1.0);
    color = mix(vec4(fogColor, 1.0), color, factor);
}
