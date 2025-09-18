// Shader - https://www.shadertoy.com/view/t3dGWj

#version 410 core
uniform vec3 iResolution;
uniform float iTime;
uniform sampler1D pcmSampleTexture;

out vec4 outColor;

vec3 colorA = vec3(0.980, 0.980, 0.945);
vec3 colorB = vec3(0.255, 0.408, 0.882);

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float y = 1.- step(abs(uv.y - (texture(pcmSampleTexture, uv.x * 0.5 + 0.5).x)), 1.f);
    float toggle = step(0.0, sin(.9/0.5 + iTime));
    vec3 background = toggle == 0. ? colorA: colorB;
    vec3 wave = toggle != 0. ? colorA: colorB;
    vec3 col = mix(background, wave, y); 
    fragColor = vec4(col,1.0);
}

void main() {
    mainImage(outColor, gl_FragCoord.xy);
}