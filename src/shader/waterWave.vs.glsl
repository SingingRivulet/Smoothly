varying vec2 position;

void main(){
    position = (gl_Vertex.xy+vec2(1.0,1.0))*0.5;
    gl_Position = vec4(gl_Vertex.xy,0.0,1.0);
}
