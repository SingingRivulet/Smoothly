uniform sampler2D tex;
varying vec4 lcolor;
void main(){
    gl_TexCoord[0] = gl_MultiTexCoord0;
    vec3 normal  = normalize(gl_NormalMatrix * gl_Normal);
    //vec3 normal  = gl_Normal;
    vec3 lightDir = normalize(vec3(gl_LightSource[0].position));
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec4 diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
    lcolor =  NdotL * diffuse;
    gl_Position = ftransform();
}
