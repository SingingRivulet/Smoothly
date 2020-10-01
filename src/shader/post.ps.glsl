varying vec2 position;
varying vec2 viewPos;
uniform sampler2D tex;
uniform sampler2D depth;
uniform sampler2D normalMap;
uniform sampler2D materialMap;
uniform sampler2D posMap;
uniform sampler2D ssaoMap;
uniform vec3 camera;
uniform float waterLevel;
uniform mat4 ProjMatrix;
uniform mat4 ViewMatrix;

uniform int windowWidth;
uniform int windowHeight;

vec3 getbloom(){
    float deltaX = 2.0/float(windowWidth);
    float deltaY = 2.0/float(windowHeight);
    vec3 bloom = vec3(0.0);
    int seg = 5;
    int i = -seg;
    int j = 0;
    float f = 0.0;
    float tot = 0.0;
    for(; i <= seg; ++i){
        for(j = -seg; j <= seg; ++j){
            f = (1.1 - sqrt(float(i*i + j*j))/8.0);
            f *= f;
            tot += f;
            vec3 ocol = texture2D( tex, vec2(position.x + float(j) * deltaX, position.y + float(i) * deltaY) ).rgb;
            ocol = vec3(max(ocol.r-0.7,0.0),max(ocol.g-0.7,0.0),max(ocol.b-0.7,0.0))*2.0;
            bloom += ocol * f;
        }
    }
    bloom /= tot;
    return bloom;
}

float getSSAO(){
    float deltaX = 2.0/float(windowWidth);
    float deltaY = 2.0/float(windowHeight);
    float ssao = 0.0;
    int seg = 5;
    int i = -seg;
    int j = 0;
    float f = 0.0;
    float tot = 0.0;
    for(; i <= seg; ++i){
        for(j = -seg; j <= seg; ++j){
            f = (1.1 - sqrt(float(i*i + j*j))/8.0);
            f *= f;
            tot += f;
            float ocol = texture2D( ssaoMap, vec2(position.x + float(j) * deltaX, position.y + float(i) * deltaY) ).r;
            ssao += ocol * f;
        }
    }
    ssao /= tot;
    return ssao*0.2;
}

void main(){
    
    vec3 normal = normalize((texture2D(normalMap,position).xyz*2.0)-vec3(1.0));//法线
    vec3 pos = texture2D(posMap,position).xyz;//位置
    float dep = texture2D(depth,position).r;//深度
    
    float ssaoFactor = getSSAO();
    vec3 bloom = getbloom();
    vec4 color = texture2D(tex,position);
    vec4 matcol = texture2D(materialMap,position);
    
    color.r+=bloom.r - matcol.r*ssaoFactor;
    color.g+=bloom.g - matcol.g*ssaoFactor;
    color.b+=bloom.b - matcol.b*ssaoFactor;
    
    //水下
    float wk = clamp((camera.y - waterLevel),0.2,1.0);
    color.r *= wk;
    color.g *= wk;
    
    gl_FragColor = color;
}
