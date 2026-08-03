#version 430 compatibility
out vec4 FragColor;

// 1. Change vec2 to vec3
uniform vec3 u_color; 
uniform float u_alpha;

void main()
{
   // 2. Pass the vec3 directly; it will map to (r, g, b)
   FragColor = vec4(u_color, u_alpha); 
}