uniform sampler2D shadowMap;
uniform mat4 shadowMatrix;
uniform float shadowFactor;
uniform int shadowMapSize;
uniform sampler2D tex;
uniform sampler2D depth;
uniform sampler2D normalMap;
uniform sampler2D materialMap;
uniform sampler2D posMap;
uniform vec3 camera;
uniform float waterLevel;
uniform mat4 ProjMatrix;
uniform mat4 ViewMatrix;

uniform int windowWidth;
uniform int windowHeight;

varying mat4 PVmat;
varying vec2 position;
varying vec2 viewPos;

float getPosDepth(vec2 pos){
    return texture2D(shadowMap, pos).r;
}

float getPosShadowOri(vec3 lightView){
    float closestDepth;
    if(lightView.x<0.0 || lightView.x>1.0 || lightView.y<0.0 || lightView.y>1.0)
        closestDepth = 1.0;
    else
        closestDepth = getPosDepth(lightView.xy);
    float currentDepth = lightView.z;
    float shadow = currentDepth-0.001 < closestDepth  ? 0.0 : 1.0;
    return shadow;
}
float getPosShadow(vec3 lightView){
    if(lightView.x<0.0 || lightView.x>1.0 || lightView.y<0.0 || lightView.y>1.0)
        return 0.0;
    
    float size = float(shadowMapSize);
    float delta = 1.0/size;
    float ix = lightView.x*size;
    float iy = lightView.y*size;
    float ax = fract(ix);
    float ay = fract(iy);
    float x  = floor(ix)/size;
    float y  = floor(iy)/size;
    float x1 = getPosShadowOri(vec3(x      ,y      ,lightView.z));
    float x2 = getPosShadowOri(vec3(x+delta,y      ,lightView.z));
    float x3 = getPosShadowOri(vec3(x      ,y+delta,lightView.z));
    float x4 = getPosShadowOri(vec3(x+delta,y+delta,lightView.z));
    float x5 = mix(x1,x2,ax);
    float x6 = mix(x3,x4,ax);
    return mix(x5,x6,ay);
}

void main(){
    vec3 pos = texture2D(posMap,position).xyz;//位置
    if(abs(pos.x)>0.1 || abs(pos.y)>0.1 || abs(pos.z)>0.1){
        //变换到光源空间
        vec4 lightView4 = shadowMatrix * vec4(pos,1.0);
        vec3 lightView = lightView4.xyz / lightView4.w;
        lightView = lightView * 0.5 + 0.5;
    
        float shadow = getPosShadow(lightView);
        
        gl_FragColor = vec4(shadow,shadow,shadow,1.0);
    }else{
        gl_FragColor = vec4(0.0,0.0,0.0,1.0);
    }
}
