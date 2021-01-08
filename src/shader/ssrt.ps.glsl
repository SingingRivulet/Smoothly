varying vec2 position;
varying vec2 viewPos;
uniform sampler2D tex;
uniform sampler2D depth;
uniform sampler2D normalMap;
uniform sampler2D materialMap;
uniform sampler2D posMap;
uniform sampler2D skyMap;
uniform sampler2D ssrtConfMap;
uniform vec3 camera;
uniform float waterLevel;
uniform mat4 ProjMatrix;
uniform mat4 ViewMatrix;
uniform mat4 skyMatrix;

uniform int windowWidth;
uniform int windowHeight;

uniform int SSRTStep;

varying mat4 PVmat;

vec3 textureSky(vec3 dir){
    vec4 pos2dv4 = skyMatrix * vec4(normalize(dir),1.0);
    vec3 pos2d = pos2dv4.xyz/pos2dv4.w;
    return texture2D(skyMap , pos2d.xy* 0.5 + 0.5).rgb;
}

float randNum(vec4 co){//随机
    return fract(sin(dot(co.rgba, vec4(12.9898,78.233,95.254,75.235))) * 43758.5453)-0.5;
}

void main(){
    vec3 normal = normalize((texture2D(normalMap,position).xyz*2.0)-vec3(1.0));//法线
    vec3 pos = texture2D(posMap,position).xyz;//位置
    vec3 ssrtConf = texture2D(ssrtConfMap,position).xyz;
    float rayMarchStepLen = 0.2;
    vec3 rayMarchPos = pos;
    vec3 reflectColor = vec3(0.0);
    if(abs(pos.x)>0.1 || abs(pos.y)>0.1 || abs(pos.z)>0.1){
    
        vec3 ray_dir = normalize(pos-camera);
        float blur = ssrtConf.x*0.25;
        vec3 randCube = vec3( 
            randNum(vec4(pos,1.0)) , 
            randNum(vec4(pos,10.0)) , 
            randNum(vec4(pos,100.0)) )*blur;
        ray_dir += randCube;
        ray_dir = normalize(ray_dir);
        
        vec3 ray_reflect = normalize(reflect(ray_dir,normal));//反射光线
        bool haveColl = false;
        vec3 rayScreenPosResult;
        if(dot(ray_dir,ray_reflect)>0.0){//没有向摄像机方向反射
            rayMarchPos += ray_reflect*rayMarchStepLen;
            bool colling = false;
            for(int i=0;i<SSRTStep;++i){
                vec4 rayScreenPos4 = PVmat * vec4(rayMarchPos,1.0);//透视
                vec3 rayScreenPos = rayScreenPos4.xyz/rayScreenPos4.w;//归一化
                rayScreenPos = rayScreenPos * 0.5 + vec3(0.5);//变换到纹理空间
                if(rayScreenPos.x<0.0 || rayScreenPos.y<0.0 || rayScreenPos.x>1.0 || rayScreenPos.y>1.0){//屏幕外
                    haveColl = false;
                    reflectColor = textureSky(ray_reflect);
                    break;
                }
                if(length(texture2D(posMap,rayScreenPos.xy).rgb-camera)<length(rayMarchPos-camera)){//发生碰撞
                    haveColl = true;
                    rayScreenPosResult = rayScreenPos;
                    rayMarchStepLen*=0.5;
                    rayMarchPos -= ray_reflect*rayMarchStepLen;
                    colling = true;
                }else{
                    if(!colling){
                        rayMarchStepLen*=2.0;
                    }
                    rayMarchPos += ray_reflect*rayMarchStepLen;
                }
            }
        }else{
            reflectColor = textureSky(ray_reflect);
        }
        if(haveColl)
            reflectColor = texture2D( tex , rayScreenPosResult.xy).rgb*(1.0-dot(-ray_dir,normal));
    }
    gl_FragColor = vec4(reflectColor,1.0);
}
