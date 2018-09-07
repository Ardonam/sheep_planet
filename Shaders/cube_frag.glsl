#version 330

in vec3 fcolor;
out vec4 color_out;
uniform vec3 lightPos;
in vec3 uPos;
in vec3 uNorm;
float ambient = .1;

void main(){
        vec3 L = normalize(lightPos - uPos);
        color_out = vec4(fcolor, 1) * (dot(uNorm, L) + ambient) ;

}




/*
uniform sampler2D tex;

in vec3 fcolor;
in vec2 fuv;

out vec4 color_out;
uniform vec3 lightPos;
in vec3 uPos;
in vec3 uNorm;
float ambient = .5;

void main() {
  vec3 L = normalize(lightPos - uPos);
  color_out = texture(tex,fuv) * vec4(fcolor, 1) * (dot(uNorm, L) + ambient);
//  color_out = vec4(fuv,0,1);
}


void main(){
        vec3 L = normalize(lightPos - uPos);
        color_out = vec4(fcolor, 1) * (dot(uNorm, L) + ambient) ;

}
*/
