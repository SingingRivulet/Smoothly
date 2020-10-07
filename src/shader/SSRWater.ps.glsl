uniform mat4 modelMatrix;
uniform float Time;
uniform float WindForce;
uniform vec2 WindDirection;
uniform float WaveLength;

varying vec4 pointPosition;
varying vec3 onormal;

void main(){
    
    vec4 color = vec4(0.0,0.0,0.2,1.0);
    gl_FragData[0] = color;
    gl_FragData[1] = color;
    gl_FragData[2] = vec4(onormal*0.5 + vec3(0.5,0.5,0.5),1.0);
    gl_FragData[3] = vec4(pointPosition.xyz/pointPosition.w,1.0);
    gl_FragData[4] = vec4(1.0,1.0,1.0,1.0);
}

