#version 460 


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
	//color = vec4(smoothstep(0.,1.,vPos+0.5),(sin(time - PI *0.5)+1.)*0.5);
	//color = texture(texture1,(vUV + time * 0.3)*(sin(time- PI *0.5)+2)*0.5);
	vec2 res = vec2(1080.0,720.0);
	vec2 frag_uv = (gl_FragCoord.xy / res ) * 2.0 -1.0;
	color = vec4(vPos,1.0);

};