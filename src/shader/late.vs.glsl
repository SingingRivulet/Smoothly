varying vec2 position;
void main(){
    position = gl_Vertex.xy;
    gl_Position = vec4(gl_Vertex.xy,0.0,1.0);
}
