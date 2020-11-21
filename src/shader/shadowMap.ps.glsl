uniform sampler2D shadowMap;
uniform mat4 shadowMatrix;
uniform float shadowFactor;
uniform sampler2D tex;
uniform sampler2D depth;
uniform sampler2D normalMap;
uniform sampler2D materialMap;
uniform sampler2D posMap;
uniform vec3 camera;
uniform float waterLevel;
uniform mat4 ProjMatrix;
uniform mat4 ViewMatrix;

uniform float windowWidth;
uniform float windowHeight;

varying mat4 PVmat;
varying vec2 position;
varying vec2 viewPos;

void main(){
    vec3 pos = texture2D(posMap,position).xyz;//位置
    if(abs(pos.x)>0.1 || abs(pos.y)>0.1 || abs(pos.z)>0.1){
        //变换到光源空间
        vec4 lightView4 = shadowMatrix * vec4(pos,1.0);
        vec3 lightView = lightView4.xyz / lightView4.w;
        lightView = lightView * 0.5 + 0.5;
    
        float closestDepth;
        if(lightView.x<0.0 || lightView.x>1.0 || lightView.y<0.0 || lightView.y>1.0)
            closestDepth = 1.0;
        else
            closestDepth = texture2D(shadowMap, lightView.xy).r;
        float currentDepth = lightView.z;
        float shadow = currentDepth-0.001 < closestDepth  ? 0.0 : 1.0;
        gl_FragColor = vec4(shadow,shadow,shadow,1.0);
    }else{
        gl_FragColor = vec4(0.0,0.0,0.0,1.0);
    }
}
