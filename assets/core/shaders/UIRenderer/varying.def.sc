// 输出布局
vec2 v_texcoord0 : TEXCOORD1 = vec2(0.0, 0.0);
vec4 v_color0    : COLOR1    = vec4(1.0, 0.0, 0.0, 1.0);
vec2 v_size_pixel      : TEXCOORD2 = vec2(0.0, 0.0);
float v_edge_pixel     : TEXCOORD3 = 0.0;
float v_corner_radius_pixel   : TEXCOORD4 = 0.0;

// 输入布局
vec3 a_position  : POSITION;
vec2 a_texcoord0 : TEXCOORD0;
vec4 a_color0    : COLOR0;

