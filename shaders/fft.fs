// Shader - https://www.shadertoy.com/view/t3dGWj

#version 410 core
uniform vec3 iResolution;
uniform float iTime;
uniform sampler1D fftSampleTexture;

out vec4 outColor;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float y =  step(uv.y  + ((texture(fftSampleTexture, uv.x).x)), 0.0f);
    vec3 col = vec3(y * (vec3(0,2,4))); 
    fragColor = vec4(col,1.0);
}


void main() {
    mainImage(outColor, gl_FragCoord.xy);
}