#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

// We will pass these simple numbers from C++
uniform vec2 u_offset;
uniform float u_angle; // In radians
uniform float u_scale;

void main() {
   // 1. Scale
   vec2 scaledPos = aPos.xy * u_scale;
   
   // 2. Rotate (Standard 2D rotation matrix math)
   float s = sin(u_angle);
   float c = cos(u_angle);
   vec2 rotatedPos = vec2(
   scaledPos.x * c - scaledPos.y * s,
   scaledPos.x * s + scaledPos.y * c
   );
   
   // 3. Translate (Move)
   vec2 finalPos = rotatedPos + u_offset;
   
   gl_Position = vec4(finalPos, 0.0, 1.0);
   ourColor = aColor;
}