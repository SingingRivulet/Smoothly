varying vec3 camera;
varying vec3 pointPosition;//坐标
varying vec4 screen;
uniform int  time;
uniform float cloudThre;//云阈值
uniform float cloudy;   //阴天
uniform float lightness;//亮度
uniform vec3  astronomical;//天体向量
uniform vec3  astrLight;//天体亮度
uniform float astrAtomScat;//大气层散射
uniform float astrViewTheta;//天体在视野中夹角
uniform vec3  astrColor;//天体颜色

float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}
float fbm(vec3 x) {
        float v = 0.0;
        //float a = 0.5;
        //vec3 shift = vec3(100);
        //for (int i = 0; i < 8; ++i) {
        //        v += a * noise(x);
        //        x = x * 2.0 + shift;
        //        a *= 0.5;
        //}
        float tm = float(time)*0.00008;
        v+=noise(x*2.0+tm)*0.5;
        v+=noise(x*4.0+tm)*0.25;
        v+=noise(x*8.0+tm)*0.125;
        v+=noise(x*16.0+tm)*0.0625;
        v+=noise(x*32.0+tm)*0.03125;
        v+=noise(x*64.0+tm)*0.015625;
        v+=noise(x*128.0+tm)*0.0078125;
        return v;
}
float cloudFilter(float x){
    return 1.0/exp(abs(x));
}
float haveCloud(vec3 p){
    //float cl = snoise2d(p.xz/100000.0)*2.0;
    //cl = snoise(p/100000.0)*cl*128.0;

    float r = fbm(vec3(p.x+float(time*100),p.y,p.z)/100000.0);
    r*=cloudFilter((p.y-24000.0)*0.000001);
    r*=cloudFilter(length(p.xz)*0.0000006);
    return r;
}

//光线步进
vec3 rayMarch(vec3 start,vec3 dir,int step,float stepLen){//dir要先normalize
    vec3 res = vec3(0.23,0.41,0.72);
    vec3 cdy = vec3(0.61,0.61,0.61);

    float ctheta = dot(normalize(astronomical),dir);
    float astRad = acos(ctheta);
    float astTheta = 1.0/exp(astRad);//视线与天体夹角
    if(astRad<astrViewTheta){
        res = astrColor;
    }else{
        res+=astrLight*astTheta*(cloudy+astrAtomScat);
    }

    vec3 dc  = cdy-res;
    res+=dc*cloudy;
    res*=lightness;

    if(dir.y<=0.0)//地平线以下
        return res;
    vec3 nvec;//当前所在的坐标
    float deltaHor = 20000.0 - dir.y;   //20000是光线步进的起始平面

    //计算起始点
    if(deltaHor>0.0){//人在云层下
        float stepDeltaHor = deltaHor/dir.y;
        nvec = start+stepDeltaHor*dir;
    }else{
        nvec = start;
    }

    vec3 deltaStep = dir*(stepLen/dir.y);//每步走的距离

    //开始光线步进
    for(int i=0;i<step;++i){
        float c = haveCloud(nvec);
        if(c>cloudThre){
            float f = exp((nvec.y-40000.0)/60000.0)*lightness;
            vec3 col =  vec3(f,f,f)+astrLight*astTheta*f;
            vec3 dt = res - col;
            res = res - dt/32.0;
        }
        nvec  += deltaStep;
    }
    return res;
}
void main(){
    vec3 ncmdir = normalize(pointPosition-camera);
    gl_FragColor = vec4(rayMarch(camera , ncmdir , 128 , 300.0).xyz*0.5,1.0);
}
