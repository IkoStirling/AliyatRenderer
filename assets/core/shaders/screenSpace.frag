#version 460 core


in vec3 vColor; 
in vec3 vPos; 
in vec2 vUV; 
out vec4 color; 

uniform float PI;
uniform float time;
uniform vec2 iResolution;
uniform sampler2D texture1;

void main(void) 
{
    vec2 res = vec2(1080.0,720.0);
    //color = texture(texture1, gl_FragCoord.xy / res);
    vec2 frag_uv = (gl_FragCoord.xy / res ) * 2.0 -1.0;
    float crossThickness = 0.002;
    float crossLength = 0.05;
    if((abs(frag_uv.x)<crossThickness && abs(frag_uv.y)<crossLength) || (abs(frag_uv.y)<crossThickness&& abs(frag_uv.x)<(crossLength / 1080.0 * 720.0)) )
    {
        color = vec4(1.0,1.0,1.0,1.0);
    }
};