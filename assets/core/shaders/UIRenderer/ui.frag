#version 460 core
in vec2 TexCoord;
in vec4 Color;
out vec4 FragColor;
    
uniform sampler2D texture1;
uniform bool useTexture;

uniform bool u_isRect;
uniform vec2 u_rectSize;      // 矩形大小，比如 (1.0, 0.5)
uniform float u_cornerRadius; // 圆角半径，比如 0.1
uniform float u_strokeWidth;  // 描边宽度，比如 0.05
uniform vec4 u_fillColor;     // 填充颜色
uniform vec4 u_strokeColor;   // 描边颜色

float roundedRectSDF(vec2 p, vec2 b, float r) {
    p = abs(p) - b + r;
    float d = length(max(p, 0.0)) + min(max(p.x, p.y), 0.0) - r;
    return d;
}

void main() 
{
    if(useTexture)
    {
        float alpha = texture(texture1, TexCoord).r;
        FragColor = vec4(Color.rgb * texture(texture1, TexCoord).r, alpha);
    }
    else
        FragColor = Color;
    if (FragColor.a < 0.1)
        discard;
}