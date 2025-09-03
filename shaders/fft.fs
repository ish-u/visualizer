#version 410 core
uniform vec3 iResolution;
uniform float iTime;
uniform sampler1D fftSampleTexture;

out vec4 outColor;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (fragCoord) / iResolution.xy;
    float y =  step(uv.y, ((texture(fftSampleTexture, uv.x).x)));
    vec3 col = vec3(y); 
    fragColor = vec4(col,1.0);
}


void main() {
    mainImage(outColor, gl_FragCoord.xy);
}