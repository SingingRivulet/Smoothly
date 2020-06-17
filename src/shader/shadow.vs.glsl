uniform mat4 shadowMatrix;
uniform mat4 modelMatrix;
void main(){
    gl_Position =shadowMatrix * modelMatrix * gl_Vertex;
}
