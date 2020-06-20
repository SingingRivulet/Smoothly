varying vec3 sposi;
varying vec4 color;
void main(){
    gl_Position = ftransform();
    sposi = gl_Position.xyz/gl_Position.w;
    color = gl_Color;
}
