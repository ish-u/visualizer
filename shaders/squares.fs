#version 410 core
uniform vec3 iResolution;
uniform float iTime;
uniform sampler1D pcmSampleTexture;

out vec4 outColor;

// https://www.shadertoy.com/view/DlGBDm

// Ref - https://www.shadertoy.com/view/3lVGWt
const float PI = 3.14;
mat2 rotationMatrix(float angle)
{
	angle *= PI / 180.0;
    float s=sin(angle), c=cos(angle);
    return mat2( c, -s, 
                 s,  c );
}

// Red - https://www.shadertoy.com/view/4llXD7
float sdBox( in vec2 p, in vec2 b )
{
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Using improvements suggested by - https://www.shadertoy.com/user/FabriceNeyret2
    
    vec2 uv = ( 2.*fragCoord - iResolution.xy ) / iResolution.y;
    uv *= rotationMatrix(iTime*20.0);
    uv = fract(uv*3.0) - 0.5;
    float d =  smoothstep(9./iResolution.y,0.0,sdBox(uv, vec2(abs(sin(iTime))* 0.4)));
    fragColor = vec4(d);
    

    /*
    vec2 uv = (fragCoord/iResolution.xy * 2.0) - 1.0;
    uv.x *= iResolution.x / iResolution.y;
     
    uv *= rotationMatrix(iTime*20.0);
    uv = fract(uv*3.0) - 0.5;

    //float d = smoothstep(0.0,0.01,sdBox(uv, vec2(0.5,0.5)));
    float d =  smoothstep(-0.1,0.0,sdBox(uv, vec2(abs(sin(iTime))* 0.4, abs(sin(iTime)) * 0.4)));
    vec3 col = (d <= 0.5 ? vec3(1.0,1.0,1.0) :vec3(0.0,0.0,0.0)) ;

    fragColor = vec4(col,1.0);
   
    */
    
}

void main() {
    mainImage(outColor, gl_FragCoord.xy);
}