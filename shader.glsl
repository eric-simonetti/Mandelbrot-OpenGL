#version 150

in vec4 vPosition;
in float vColor;
out vec3 color;

void main(){
    gl_Position = vPosition;
    if((vColor - 0.0) < 0.001) color = vec3(vColor, 1.0, 0.0);
    else color = vec3(vColor, 1.0, 1.0);
}
