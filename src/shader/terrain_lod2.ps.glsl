uniform sampler2D tex;
uniform sampler2D shadowMap;
uniform mat4 shadowMatrix;
uniform mat4 modelMatrix;
uniform float shadowFactor;
uniform vec3 campos;

uniform float scan_animation_showing;
uniform float scan_animation_size;

varying vec3 pointPosition;//坐标
varying vec4 pointPosition4;
varying float temp;//温度
varying float humi;//湿度

varying vec3 normal;
varying vec3 onormal;
varying vec3 lightDir;


vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

float snoise(vec3 v){
    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy) );
    vec3 x0 =   v - i + dot(i, C.xxx) ;

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );

    //  x0 = x0 - 0. + 0.0 * C
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + 2.0 * C.xxx;
    vec3 x3 = x0 - 1. + 3.0 * C.xxx;

    // Permutations
    i = mod(i, 289.0 );
    vec4 p = permute( permute( permute(
                                   i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
                               + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
                      + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

    // Gradients
    // ( N*N points uniformly over a square, mapped onto an octahedron.)
    float n_ = 1.0/7.0; // N=7
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );

    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);

    //Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                  dot(p2,x2), dot(p3,x3) ) );
}

vec3 permute2d(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }
float snoise2d(vec2 v){
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute2d( permute2d( i.y + vec3(0.0, i1.y, 1.0 ))
  + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

// Ridged multifractal
// See "Texturing & Modeling, A Procedural Approach", Chapter 12
float ridge(float h, float offset) {
    h = abs(h);     // create creases
    h = offset - h; // invert so creases are at top
    h = h * h;      // sharpen creases
    return h;
}

float turbulence (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < 3; i++) {
        //snoise : 2D simplex noise function
        value += amplitude * abs(snoise2d(st));
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}
void main(){
    float n1                    = snoise(pointPosition*10.0);
    float n2                    = snoise(pointPosition*20.0);
    float noise                 = (n1+n2)*2.0+0.5;
    //float texture_delta_fissure = turbulence(pointPosition.xz*0.1);
    vec3  texture_grass         = vec3(0.1,0.7,0.1)*noise;
    vec3  texture_mud           = vec3(0.9,0.7,0.4)*noise;
    vec3  texture_sand          = vec3(0.5,0.3,0.1)*noise;
    vec3  texture_snow          = vec3(1.2,1.2,1.2)*noise;
    vec4  diffuseColor;
    float theta                 = snoise(pointPosition);

    vec3 padd;
    float sand_theta = (theta*16.0+2.0) - pointPosition.y;
    if(sand_theta<0.0)
        padd = texture_mud;
    else
        padd = mix(texture_mud,texture_sand,min(sand_theta,1.0));

    if(temp>theta*2.0+0.5){
        float grass_theta = onormal.y-(theta*2.0+0.6);
        if(grass_theta>0.0){
            diffuseColor = vec4(mix(padd,texture_grass,min(grass_theta*10.0,1.0)),1.0);
        }else{
            diffuseColor = vec4(padd,1.0);
        }
    }else{
        diffuseColor = vec4(texture_snow,1.0);
    }

    vec4 diffuse;
    float NdotL;

    vec3 rnormal = normal;
    rnormal+=vec3(n1 , snoise(pointPosition+vec3(0.0,100.0,0.0)), snoise(pointPosition+vec3(0.0,400.0,100.0)))*4.0;
    rnormal+=vec3(n2 , snoise(pointPosition*10.0+vec3(0.0,100.0,0.0)), snoise(pointPosition*10.0+vec3(0.0,400.0,100.0)))*4.0;
    rnormal=normalize(rnormal);

    NdotL = max(dot(rnormal, lightDir), 0.0);
    diffuse = gl_LightSource[0].diffuse;
    vec4 lcolor =  NdotL * diffuse;
    lcolor.x +=0.1;
    lcolor.y +=0.1;
    lcolor.z +=0.1;

    vec4 scolor = vec4(diffuseColor.x*lcolor.x , diffuseColor.y*lcolor.y , diffuseColor.z*lcolor.z ,1.0);
    
    vec4 lightView4 = shadowMatrix * pointPosition4;
    vec3 lightView = lightView4.xyz / lightView4.w;
    lightView = lightView * 0.5 + 0.5;
    
    float closestDepth;
    if(lightView.x<0.0 || lightView.x>1.0 || lightView.y<0.0 || lightView.y>1.0)
        closestDepth = 1.0;
    else
        closestDepth = texture2D(shadowMap, lightView.xy).r;
    float currentDepth = lightView.z;
    float shadow = currentDepth-0.001 < closestDepth  ? 0.0 : 1.0;
    
    scolor -= vec4(scolor.rgb,0.0)*shadow*shadowFactor;

    scolor.b += scan_animation_showing/exp(abs(length(pointPosition-campos)-scan_animation_size));

    gl_FragColor = scolor;

}
