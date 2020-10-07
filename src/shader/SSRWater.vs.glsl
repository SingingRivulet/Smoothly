uniform mat4 modelMatrix;
uniform float Time;
uniform float WindForce;
uniform vec2 WindDirection;
uniform float WaveLength;

varying vec4 pointPosition;
varying vec2 bumpMapTexCoord;
varying vec3 onormal;

void main(){
    gl_TexCoord[0] = gl_MultiTexCoord0;
    pointPosition = modelMatrix * gl_Vertex;
    gl_Position = ftransform();
    bumpMapTexCoord = gl_MultiTexCoord0.xy / 0.1 + Time * WindForce * WindDirection;
    onormal  = normalize(modelMatrix * vec4(gl_Normal,0.0)).xyz;
}
