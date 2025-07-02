	#version 460 core
    layout(location=0) in vec3 aPos;
    layout(location=1) in vec4 aColor;
    layout(location=2) in vec2 auv;

    uniform mat4 projection;
    uniform mat4 view;

    out vec4 v_Color;

    void main() 
    {
        gl_Position = projection * view *  vec4(aPos, 1.0);
        v_Color = aColor;
    }