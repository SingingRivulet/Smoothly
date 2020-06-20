varying vec3 sposi;
varying vec4 color;
void main(){
    gl_FragColor =color;
    gl_FragColor.a = max(1.0-sqrt(sposi.x*sposi.x+sposi.y*sposi.y) , 0.0);
}
