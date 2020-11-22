uniform sampler2D shadowViewMap;
uniform float shadowFactor;
uniform vec3 lightDir;
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

float randNum(vec3 co){
    return fract(sin(dot(co.xyz, vec3(12.9898,78.233,65.4323))) * 43758.5453)-0.5;
}

float getPosShadow(vec3 pos){
    vec4 otp = PVmat * vec4(pos,1.0);//透视变换
    vec3 tp = otp.xyz/otp.w;//归一化
    tp = tp * 0.5 + vec3(0.5);//变换到纹理空间
    if(tp.x<0.0 || tp.y<0.0 || tp.x>1.0 || tp.y>1.0)
        return 0.0;
    vec3 samplePos = texture2D(posMap,tp.xy).rgb;
    if(abs(samplePos.x)>0.1 || abs(samplePos.y)>0.1 || abs(samplePos.z)>0.1){
        if(length(samplePos-pos)<0.1)
            return texture2D(shadowViewMap,tp.xy).r;
    }
    return 0.0;
}

vec3 genSampleNormal(vec3 v_normal,vec3 rotNormal){
    vec3 N = normalize(v_normal);
    vec3 v_tangent = cross(N,vec3(0.0,1.0,0.0));
    vec3 T = normalize(v_tangent- dot(v_normal,v_tangent)*v_normal);//对T进行修正，因为T与B和N可能不垂直
    vec3 B = cross(N,T);
    mat3 TBN= mat3(T,B,N);
 
    vec3 normal = 2.0*rotNormal-vec3(1.0,1.0,1.0); //转换范围到（-1,1）
 
    normal = normalize(TBN*normal);
    
    return normal;
}

void main(){
    vec3 normal = normalize((texture2D(normalMap,position).xyz*2.0)-vec3(1.0));//法线
    vec3 pos = texture2D(posMap,position).xyz;//位置
    float dep = texture2D(depth,position).r;//深度
    
    float shadowSum = 0.0;
    if(abs(pos.x)>0.1 || abs(pos.y)>0.1 || abs(pos.z)>0.1){
        for(int i=0;i<8;++i){
            vec3 rot = normalize( vec3( randNum(vec3(position,1.0*float(i))) , randNum(vec3(position,10.0*float(i))) , 1.0 ) );
            float sd = getPosShadow(pos+genSampleNormal(normal, rot )*0.1);
            shadowSum += sd;
        }
    }

    gl_FragColor = vec4(shadowSum/8.0,0.0,0.0,1.0);
}
