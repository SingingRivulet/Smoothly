varying vec2 position;
varying vec2 viewPos;
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


float randNum(vec2 co){//随机
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453)-0.5;
}

void main(){
    vec3 normal = normalize((texture2D(normalMap,position).xyz*2.0)-vec3(1.0));//法线
    vec3 pos = texture2D(posMap,position).xyz;//位置
    float rayMarchStepLen = 0.2;
    vec3 rayMarchPos = pos;
    vec3 reflectColor = vec3(0.0);
    if(abs(pos.x)>0.1 && abs(pos.y)>0.1 && abs(pos.z)>0.1){
        vec3 ray_dir = normalize(pos-camera);
        vec3 ray_reflect = normalize(reflect(ray_dir,normal));//反射光线
        bool haveColl = false;
        vec3 rayScreenPosResult;
        if(dot(ray_dir,ray_reflect)>0.0){//没有向摄像机方向反射
            rayMarchPos += ray_reflect*rayMarchStepLen;
            for(int i=0;i<64;++i){
                vec4 rayScreenPos4 = PVmat * vec4(rayMarchPos,1.0);//透视
                vec3 rayScreenPos = rayScreenPos4.xyz/rayScreenPos4.w;//归一化
                rayScreenPos = rayScreenPos * 0.5 + vec3(0.5);//变换到纹理空间
                if(rayScreenPos.x<0.0 || rayScreenPos.y<0.0 || rayScreenPos.x>1.0 || rayScreenPos.y>1.0){//屏幕外
                    haveColl = false;
                    break;
                }
                if(length(texture2D(posMap,rayScreenPos.xy).rgb-camera)<length(rayMarchPos-camera)){//发生碰撞
                    haveColl = true;
                    rayScreenPosResult = rayScreenPos;
                    rayMarchStepLen*=0.5;
                    rayMarchPos -= ray_reflect*rayMarchStepLen;
                }else{
                    rayMarchPos += ray_reflect*rayMarchStepLen;
                }
            }
        }
        if(haveColl)
            reflectColor = texture2D( tex , rayScreenPosResult.xy).rgb;
    }
    gl_FragColor = vec4(reflectColor,1.0);
}
