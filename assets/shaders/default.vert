#version 430 compatibility
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec4 vColor;

uniform vec2 u_offset;
uniform float u_scale;
uniform float u_aspect;
uniform float u_sinAngle;
uniform float u_cosAngle;
uniform vec3 u_color;
uniform float u_alpha;

void main() {
   // 1. Scale
   vec2 scaledPos = aPos.xy * u_scale;
   
   // 2. Rotate (Standard 2D rotation matrix math)
   vec2 rotatedPos = vec2(
      scaledPos.x * u_cosAngle - scaledPos.y * u_sinAngle,
      scaledPos.x * u_sinAngle + scaledPos.y * u_cosAngle
   );
   
   // 3. Aspect Ratio Correction (Applied AFTER rotation!)
   rotatedPos.x *= u_aspect;
   
   // 4. Translate (Move)
   vec2 finalPos = rotatedPos + u_offset;
   gl_Position = vec4(finalPos, 0.0, 1.0);

   vColor = vec4(aColor * vec3(u_color.x, u_color.y, u_color.z), u_alpha);
}