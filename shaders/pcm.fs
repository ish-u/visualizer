// Shader - https://www.shadertoy.com/view/t3dGWj

#version 410 core
uniform vec3 iResolution;
uniform float iTime;
uniform sampler1D pcmSampleTexture;

out vec4 outColor;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    // uv.x * 0.5 + 0.5 - maps uv [-1,1] to texture [0,1]  
    float y = step(abs(texture(pcmSampleTexture, uv.x * 0.5 + 0.5).x), abs(uv.y));
    vec3 col = vec3(1. - y);
    fragColor = vec4(col,1.0);
}

void main() {
    mainImage(outColor, gl_FragCoord.xy);
}