#version 410 core
uniform vec3 iResolution;
uniform float iTime;
uniform sampler1D pcmSampleTexture;

out vec4 outColor;

// https://www.shadertoy.com/view/DtKfzG

const float PI = 3.14;
mat2 rotationMatrix(float angle)
{
	angle *= PI / 180.0;
    float s=sin(angle), c=cos(angle);
    return mat2( c, -s, 
                 s,  c );
}

vec3 palette(float t)
{    
    vec3 a = vec3(0.049, 0.109, 0.662);
    vec3 b = vec3(0.408, 0.456 ,0.077);
    vec3 c = vec3(0.564, 0.367 ,0.556);
    vec3 d = vec3(2.722, 2.609, 0.063);

    return a + b*cos(3.14*(c*t+d) );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    
    vec2 uv = (2.0*fragCoord - iResolution.xy) / iResolution.y;
    uv *= rotationMatrix(iTime*25.0);
    uv = (fract(uv * 2.0) * 3.0) - 1.5;
    uv *= rotationMatrix(-1.0*iTime*100.0);    
    float d = (pow(uv.x, 2.) + pow(uv.y, 2.) - uv.y * abs(uv.x));
    d = exp(sin(d)) + iTime*0.8 + d;
    vec3 col = smoothstep(0.0,9./iResolution.y,palette(d));
    fragColor = vec4(col,1.0);
    
}

void main() {
    mainImage(outColor, gl_FragCoord.xy);
}