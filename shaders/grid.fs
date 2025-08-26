#version 410 core
uniform vec3 iResolution;
uniform float iTime;
uniform sampler1D pcmSampleTexture;

out vec4 outColor;

// Ref - https://www.shadertoy.com/view/4llXD7
float sdBox( in vec2 p, in vec2 b )
{
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

// Ref - https://www.shadertoy.com/view/Nt23Dc
vec3 hash32(vec2 p) {
	vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz+33.33);
    return fract((p3.xxy+p3.yzz)*p3.zyx);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float blur = 1.5/iResolution.y;
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    vec2 uv0 = uv;
    uv *= 4.;
    uv = fract(uv);
    
    vec2 boxId = ceil(uv0* 4.);
    vec3 colorA = hash32(boxId);
    vec3 colorB = hash32(1. - boxId);
    float blend = abs(sin(colorA.x * colorA.y + iTime));
    vec3 baseColor = mix(colorA,colorB, blend);

    float box = sdBox(uv - vec2(0.5),vec2(0.49));
    box = smoothstep(blur,0.,box);
    vec3 boxColor = box * baseColor;
    
    float circleRadius = 0.05;
    float circle = 1.- smoothstep(circleRadius, circleRadius + blur ,distance(uv, vec2(0.0,0.0)));
    circle += 1.- smoothstep(circleRadius, circleRadius + blur ,distance(uv, vec2(1.0,1.0)));
    circle += 1.- smoothstep(circleRadius, circleRadius + blur ,distance(uv, vec2(0.0,1.0)));
    circle += 1.- smoothstep(circleRadius, circleRadius + blur ,distance(uv, vec2(1.0,0.0)));

    vec3 circleColor = mix(vec3(0.),vec3(1.), 1.);

    fragColor = vec4(mix(boxColor,circleColor, circle),1.0);
} 


void main() {
    mainImage(outColor, gl_FragCoord.xy);
}