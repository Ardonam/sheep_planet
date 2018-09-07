#version 330

uniform mat4 view;
in vec3 fcolor;
out vec4 color_out;
uniform vec3 lightPos;
in vec3 uPos;
in vec3 camPos;
in vec3 uNorm;
uniform float ambient;
uniform float shininess;
uniform float speck ;


void main(){
        vec3 N = normalize(uNorm);
        vec3 L = normalize(lightPos - uPos);
        vec3 R = 2*dot(N,L)*N - L;
        vec3 V = normalize(camPos- uPos);
        color_out = vec4(fcolor * (dot(uNorm, L) + ambient) + (vec3(1,1,1) * speck *pow(clamp(dot(R,V),0,1),shininess)), 1) ;

}
