varying vec3 normal;
varying vec3 lightDir;
void main(){
    vec4 diffuse;
    float NdotL;

    vec3 rnormal = normal;
    rnormal=normalize(rnormal);

    NdotL = max(dot(rnormal, lightDir), 0.0);
    diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
    vec4 lcolor =  NdotL * diffuse;

    vec4 scolor = vec4(lcolor.x*0.1 , lcolor.y*0.7 , lcolor.z*0.1 ,1.0);

    gl_FragColor = scolor;
}
