uniform sampler2D tex;
uniform mat4 shadowMatrix;
uniform mat4 modelMatrix;
uniform mat4 transformMatrix;

varying vec4 lcolor;
varying vec4 pointPosition;
varying float NdotL;
varying vec3 normal;
varying vec3 onormal;

void main(){
    gl_TexCoord[0] = gl_MultiTexCoord0;
    normal  = normalize(gl_NormalMatrix * gl_Normal);
    onormal  = normalize(modelMatrix * vec4(gl_Normal,0.0)).xyz;
    vec3 lightDir = normalize(vec3(gl_LightSource[0].position));
    NdotL = max(dot(normal, lightDir), 0.0);
    vec4 diffuse = gl_LightSource[0].diffuse;
    lcolor =  NdotL * diffuse;
    pointPosition = modelMatrix * gl_Vertex;
    gl_Position = ftransform();
}
