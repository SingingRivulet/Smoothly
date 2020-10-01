varying vec2 position;
varying vec2 viewPos;
void main(){
    position = (gl_Vertex.xy+vec2(1.0,1.0))*0.5;
    gl_Position = vec4(gl_Vertex.xy,0.0,1.0);
    viewPos = gl_Vertex.xy;
}
