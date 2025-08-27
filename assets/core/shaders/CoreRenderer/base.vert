	#version 460 core
    layout(location=0) in vec3 aPos;
    layout(location=1) in vec4 aColor;
    layout(location=2) in vec2 auv;

    uniform mat4 u_projection;
    uniform mat4 u_view;

    out vec4 v_color;

    void main() 
    {
        gl_Position = u_projection * u_view *  vec4(aPos, 1.0);
        v_color = aColor;
    }