#version 330



uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;


in vec3 position;
in vec3 normal;
in vec3 color;
out vec3 fcolor;
out vec3 uPos;
out vec3 uNorm;



void main() {
  gl_Position = projection * view * model  * vec4(position, 1);
  uPos = (model * vec4(position, 1)).xyz;
  uNorm = (transpose(inverse(model)) * vec4(normal, 0)).xyz;
  fcolor = color;
}
