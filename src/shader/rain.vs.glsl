uniform sampler2D tex;
uniform sampler2D shadowMap;
uniform mat4 shadowMatrix;
uniform mat4 modelMatrix;
uniform mat4 transformMatrix;

varying vec4 pointPosition;

void main(){
    gl_TexCoord[0] = gl_MultiTexCoord0;
    pointPosition = gl_ModelViewMatrix * vec4(0.0, 0.0, 0.0, 1.0);
    gl_Position = gl_ProjectionMatrix * (pointPosition + vec4(gl_Vertex.x, gl_Vertex.y, 0.0, 0.0));
}
