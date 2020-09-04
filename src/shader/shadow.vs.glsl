uniform mat4 shadowMatrix;
uniform mat4 modelMatrix;
void main(){
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position =shadowMatrix * modelMatrix * gl_Vertex;
}
