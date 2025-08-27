#version 460 core
in vec2 TexCoord;
in vec4 Color;
out vec4 FragColor;
    
uniform sampler2D texture1;
uniform bool useTexture;
    
void main() 
{
    if(useTexture)
        FragColor = texture(texture1, TexCoord) * Color;
    else
        FragColor = Color;
    if (FragColor.a < 0.1)
        discard;
}