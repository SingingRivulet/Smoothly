uniform sampler2D tex;
uniform sampler2D shadowMap;
uniform mat4 shadowMatrix;
uniform mat4 modelMatrix;
uniform mat4 transformMatrix;

varying vec4 lcolor;
varying vec4 pointPosition;
varying float NdotL;
varying vec3 normal;

void main(){
    gl_TexCoord[0] = gl_MultiTexCoord0;
    normal  = normalize(gl_NormalMatrix * gl_Normal);
    vec3 lightDir = normalize(vec3(gl_LightSource[0].position));
    NdotL = max(dot(normal, lightDir), 0.0);
    vec4 diffuse = gl_LightSource[0].diffuse;
    lcolor =  NdotL * diffuse;
    pointPosition = modelMatrix * gl_Vertex;
    gl_Position = ftransform();
}
