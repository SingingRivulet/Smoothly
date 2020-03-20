varying vec3 camera;
varying vec3 pointPosition;//坐标
varying vec4 screen;
uniform int  time;

float t = 100.0/60.0;

float function 			= mod(t,4.0);
bool  multiply_by_F1	= mod(t,8.0)  >= 4.0;
bool  inverse				= mod(t,16.0) >= 8.0;
float distance_type	= mod(t/16.0,4.0);

vec2 hash( vec2 p ){
        p = vec2( dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3)));
        return fract(sin(p)*43758.5453);
}

float voronoi( in vec2 x ){
    vec2 n = floor( x );
    vec2 f = fract( x );

    float F1 = 8.0;
    float F2 = 8.0;

    for( int j=-1; j<=1; j++ )
        for( int i=-1; i<=1; i++ ){
            vec2 g = vec2(i,j);
            vec2 o = hash( n + g );

            o = 0.5 + 0.41*sin( 100.0 + 6.2831*o );
            vec2 r = g - f + o;

            float d = 	distance_type < 1.0 ? dot(r,r)  :				// euclidean^2
                                              distance_type < 2.0 ? sqrt(dot(r,r)) :			// euclidean
                                                                    distance_type < 3.0 ? abs(r.x) + abs(r.y) :		// manhattan
                                                                                          distance_type < 4.0 ? max(abs(r.x), abs(r.y)) :	// chebyshev
                                                                                                                0.0;

            if( d<F1 ) {
                F2 = F1;
                F1 = d;
            } else if( d<F2 ) {
                F2 = d;
            }
        }

    float c = function < 1.0 ? F1 :
                               function < 2.0 ? F2 :
                                                function < 3.0 ? F2-F1 :
                                                                 function < 4.0 ? (F1+F2)/2.0 :
                                                                                  0.0;

    if( multiply_by_F1 )	c *= F1;
    if( inverse )			c = 1.0 - c;

    return c;
}

float fbm( in vec2 p ){
    float s = 0.0;
    float m = 0.0;
    float a = 0.5;

    for( int i=0; i<6; i++ ){
        s += a * voronoi(p);
        m += a;
        a *= 0.5;
        p *= 2.0;
    }
    return s/m;
}
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
        v+=noise(x*2.0)/2.0;
        v+=noise(x*4.0)/4.0;
        v+=noise(x*8.0)/8.0;
        v+=noise(x*16.0)/16.0;
        v+=noise(x*32.0)/32.0;
        v+=noise(x*64.0)/64.0;
        //v+=noise(x*128.0)/128.0;
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
    return r>0.5 ? 1.0:0.0;
}

//光线步进
vec3 rayMarch(vec3 start,vec3 dir,int step,float stepLen){//dir要先normalize
    vec3 res = vec3(0.23,0.41,0.72);
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
        if(c>0.5){
            float f = exp((nvec.y-40000.0)/60000.0);
            vec3 col =  vec3(f,f,f);
            vec3 dt = res - col;
            res = res - dt/2.0;
        }
        nvec  += deltaStep;
    }
    return res;
}
void main(){
    vec3 ncmdir = normalize(pointPosition-camera);
    gl_FragColor = vec4(rayMarch(camera , ncmdir , 128 , 300.0).xyz*0.5,1.0);
}
