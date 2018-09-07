#version 330
in vec3 fcolor;
out vec4 color_out;
uniform vec3 lightPos;
in vec3 uPos;
in vec3 uNorm;
float ambient = 7;

void main(){
        vec3 L = normalize(lightPos - uPos);
        color_out = vec4(fcolor, 1) * (dot(uNorm, L) + ambient) ;

}
