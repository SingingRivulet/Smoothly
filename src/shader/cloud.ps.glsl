varying vec3 camera;
varying vec3 pointPosition;//坐标
varying vec4 screen;
uniform int  time;
/*
float raySphereIntersect(vec3 start, vec3 dir , float r){
    vec3 oc = start;
    float projoc = dot(dir, oc);
    float oc2 = dot(oc, oc);
    float distance2 = oc2 - projoc * projoc;   //计算出的球心到射线的距离
    float discriminant = r*r - distance2;  //使用勾股定理，计算出另一条边的长度
    discriminant = sqrt(discriminant);
    return projoc + discriminant;
}
*/
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
        float tm = float(time)*0.00004;
        v+=noise(x*2.0+tm)/2.0;
        v+=noise(x*4.0+tm)/4.0;
        v+=noise(x*8.0+tm)/8.0;
        v+=noise(x*16.0+tm)/16.0;
        v+=noise(x*32.0+tm)/32.0;
        v+=noise(x*64.0+tm)/64.0;
        //v+=noise(x*128.0)/128.0;
        return v;
}
/*
float cloudFilter(float x){
    return 1.0/exp(abs(x));
}
float haveCloud(vec3 p){
    //float cl = snoise2d(p.xz/100000.0)*2.0;
    //cl = snoise(p/100000.0)*cl*128.0;

    float r = fbm(vec3(p.x+float(time*100),p.y,p.z)/100000.0);
    r*=cloudFilter((p.y-24000.0)*0.000001);
    r*=cloudFilter(length(p.xz)*0.0000006);
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
*/
//out vec4 color;

//uniform vec3 cameraPosition;
//uniform vec3 cameraFront;
//uniform vec3 cameraUp;
//uniform vec3 cameraRight;
// Earth center position
const vec3 EarthCenter = vec3(0.0,-6378000.0,0.0);

//uniform float screenWidth;
//uniform float screenHeight;
//uniform float Time;

// Halton Sequences
const vec4 HaltonSequence = vec4(0.0,0.0,0.0,0.0);
//uniform vec4 HaltonSequence2;
//uniform vec4 HaltonSequence3;
//uniform vec4 HaltonSequence4;

//uniform sampler3D lowFrequencyTexture;
//uniform sampler3D highFrequencyTexture;
//uniform sampler2D WeatherTexture;

//uniform sampler2D CurlNoiseTexture;

// Define Global variables used by all the program
const float MPI = 3.14159265;
const float EARTH_RADIUS = 6378000.0;
const float THICK_ATMOSPHERE = 15000.0;
const float ATMOSPHERE_INNER_RADIUS = EARTH_RADIUS + 10500.0;
const float ATMOSPHERE_OUTER_RADIUS = ATMOSPHERE_INNER_RADIUS + THICK_ATMOSPHERE;

// The Sun Location
const vec3 SunLocation = vec3(0.0, ATMOSPHERE_OUTER_RADIUS * 2.0, 2.0 * EARTH_RADIUS);
const float SunIntensity = 0.35;

// ** Definition of samplers

// 3D Texture - Return the RGBA sample point Perlin - Worley nose
vec4 SampleLowFrequencyTexture(vec3 point){
    float r = fbm(point);
    return vec4(r,r,r,r);
}
// Sample High Frequency Texture 3D RGB
vec3 SampleHighFrequencyTexture(vec3 point){
    float r = fbm(point);
    return vec3(r,r,r);
}
// Sample Weather Texture 2D RGB
vec3 SampleWeatherTexture(vec2 point){
    float r = fbm(vec3(point.xy,0.0));
    return vec3(r,r,r);
}
// Sample Curl noise texture 2D RGB
vec3 SampleCurlNoiseTexture(vec2 point){
    float r = fbm(vec3(point.xy,0.0));
    return vec3(r,r,r);
}

// ** Get ray direction definition
vec3 GetRayDirection(vec3 front, vec3 right, vec3 up, float x, float y){
    vec3 ray = 2.0 * front +  right * x + up * y;
    return normalize(ray);
}
// ** Remap function defined by the author of paper
float Remap(float original_value, float original_min, float original_max, float new_min, float new_max){
    return new_min + (((original_value - original_min)/(original_max - original_min)) * (new_max - new_min));
}

// GetHeighfraction

float GetHeightFractionForPoint(vec3 inPosition, vec2 inCloudMinMax){
    float height_fraction = (inPosition.z - inCloudMinMax.x)/(inCloudMinMax.y - inCloudMinMax.x);
    return clamp(height_fraction, 0.0, 1.0);
}

// Keep in box [0, 1]
vec3 KeepInBox(vec3 point){
    vec3 newpoint = point;
    if(point.x < 0.0)
        newpoint.x = 1.0 - point.x;
    if(point.y < 0.0)
        newpoint.y = 1.0 - point.y;
    if(point.z < 0.0)
        newpoint.z = 1.0 - point.z;

    return newpoint;
}

// Get Relative cloud in thick atmosphere
float GetRelativeHeightInAtmosphere(vec3 position, vec3 earthCenter){
    float distCenter2Point = length(position - earthCenter);
    float relativeDistance = distCenter2Point - ATMOSPHERE_INNER_RADIUS;

    return relativeDistance/THICK_ATMOSPHERE;
}

// temporal computing of height
float GetHeightInAtmosphere(vec3 pointInAtm, vec3 earthCenter, vec3 intersectionRay2InnerAtm, vec3 rayDirection, vec3 eyePos, float atmThick){
    float distanceCamera2Point = length(pointInAtm - eyePos);
    float distanceCamera2InnerAtm = length(intersectionRay2InnerAtm - eyePos);
    // Asumming module of Ray direction
    vec3 point2EarthCenter = normalize(pointInAtm - earthCenter);

    float cosThet = dot(rayDirection, point2EarthCenter);

    float posInAtm = abs(cosThet * (distanceCamera2Point - distanceCamera2InnerAtm));

    return clamp(posInAtm/atmThick, 0.0, 1.0);
}

/*
 * @brief: GetGradientHeightFactor
 * Defines the gradient height functions, up & down type sin.
 * The function parameters is defined by the cloudtype
 * cloudtype:  Type of cloud: 0-> Stratus, 1-> Cumulus, 2-> Cumulonimbus
 * Height:     Relative height [0-1] respect to the atmosphere thickness
 */

float GetGradientHeightFactor(float height, int cloudtype){
    float timewidthup, starttimeup, starttimedown;
    // Stratus clouds
    if(cloudtype == 0){
        timewidthup     = 0.08;
        starttimeup     = 0.08;
        starttimedown   = 0.2;
    } // Cumulus clouds
    else if(cloudtype == 1){
        timewidthup     = 0.14;
        starttimeup     = 0.1;
        starttimedown   = 0.5;
        }
    // Cumulunimbusclouds
    else if(cloudtype == 2){
        timewidthup     = 0.2;
        starttimeup     = 0.10;
        starttimedown   = 0.7;
    }

    float factor = 2.0 * MPI/(2.0 * timewidthup);

    float density_gradient = 0.0;
    // Gradient functions dependent on the cloudtype
    if(height < starttimeup)
        density_gradient = 0.0;
    else if(height < starttimeup + timewidthup)
        density_gradient = 0.5 * sin(factor * height - MPI/2.0 - factor * starttimeup) + 0.5;
    else if(height < starttimedown)
        density_gradient = 1.0;
    else if(height < starttimedown + timewidthup)
        density_gradient = 0.5 * sin(factor * height - MPI/2.0 - factor * (starttimedown + timewidthup)) + 0.5;
    else
        density_gradient = 0.0;

    return density_gradient;
}

// TODO: Remove argument point if it's not necessary:

/*
 * @brief: GetDensityHeightGradientForPoint
 * Return the gradient in accordance to the cloudtype and the Relative Height.
 * vec3  weather_data:   data vector for the sampling point z: cloudtype
 * float relativeHeight: Relative height respect to the atmosphere thickness
 */
float GetDensityHeightGradientForPoint(vec3 point, vec3 weather_data, float relativeHeight){
    float cloudt = weather_data.z;
    // Cloud type: {0: Stratus, 1: Cumulus, 2: Cumulonimbus}
    int cloudtype = 1;
    if(cloudt < 0.1)
        cloudtype = 0;
    else if(cloudt > 0.9)
        cloudtype = 2;

    //cloudtype = 2;
    // Relative Height from [0 - 1]
    //float relh = point.y;
    //float relativeHeight = GetRelativeHeightInAtmosphere(point, earthCenter);

    // Get gradient function defined for three type of clouds
    return GetGradientHeightFactor(relativeHeight, cloudtype);
}
// Define the intersection between ray & dome

vec3 GetIntersectionSphereRay(vec3 rayOrigin, vec3 rayDirection, vec3 sphereCenter, float sphereRadius){
    // Solve an second grade equation
    // Distance from spherecenter to ray origin
    float er = length(rayOrigin - sphereCenter);
    // Vector of direction normalized
    vec3 uer = normalize(rayOrigin - sphereCenter);
    // if Intersection = rayOrigin + rayDirection * t
    // The second order equation is:
    // er^2 + t^2 + 2*er*t*dot(uer, rayDirection) = sphereRadius^2
    float A = dot(rayDirection, rayDirection);
    float B = 2.0 * er * dot(uer, rayDirection);
    float C = er * er - sphereRadius * sphereRadius;
    // Define the discriminant
    float discriminant = B * B - 4.0 * A * C;
    // Solve for t the equation
    float t;
    if(discriminant < 0.0)
        t = 0.0;
    else{
        t = (-B - sqrt(discriminant))/(2.0 * A);
        if(t < 0.0)
            t = (-B + sqrt(discriminant))/(2.0 * A);
            if(t < 0.0)
                t = 0.0;
    }
    // Second Way test:
    /*vec3 uer = (rayOrigin - sphereCenter)/sphereRadius;
    float A = dot(rayDirection, rayDirection);
    float B = 2 * dot(uer, rayDirection);
    float C = dot(uer, uer) - 1.0;
    float discriminant = B * B - 4 * A * C;
    float t;
    if(discriminant < 0.0)
        t = 0.0;
    else{
        t = (-B - sqrt(discriminant))/(2.0 * A);
        if(t < 0.0)
            t = (-B + sqrt(discriminant))/(2.0 * A);
            if(t < 0.0){
                t = 0.0;
            }
    }*/


    return rayOrigin + t  * rayDirection;
}

// Get point sampling 3D texture

vec3 GetPositionInAtmosphere(vec3 pos, vec3 earthCenter, float thickness){
    vec3 posit = (pos - vec3(earthCenter.x, ATMOSPHERE_INNER_RADIUS - EARTH_RADIUS, earthCenter.z))/thickness;

    return posit;
}

// @brief
// Get the point of weather texture to sample

vec2 GetRelativePointToWeatherMap(vec3 positionInDome, vec3 eyePosition, vec3 domeCenter, float radiusDome){
    float r = EARTH_RADIUS;
    float dist = sqrt(radiusDome*radiusDome - r * r);
    vec3 posXmin = domeCenter + vec3(-dist, r,  0.0 );
    vec3 posZmin = domeCenter + vec3(0.0  , r, -dist);

    posXmin.y = 0.0;
    posZmin.y = 0.0;
    positionInDome.y = 0.0;

    float posx = abs(positionInDome.x - posXmin.x)/ (2.0 * dist);
    float posz = abs(positionInDome.z - posZmin.z)/ (2.0 * dist);

    //float posx = length(positionInDome - posXmin)/(2 * dist);
    //float posy = length(positionInDome - posZmiz)/(2 * dist);
    vec2 pointSample = vec2(posx, posz) * 1.00;
    return pointSample;
}

/*
 * @brief: HenyeyGreenstein function
 * This function models the atenuation of borders in light scattering
 */
float HenyeyGreenstein(in vec3 inLightVector, in vec3 inViewVector, in float g){
    // Cos of angle
    float cos_angle = dot(normalize(inLightVector), normalize(inViewVector));

    // Define the HenyeyGreenstein function
    return (1.0 - g * g)/(pow(1.0 + g * g - 2.0 * g * cos_angle, 1.50) * 4.0 * MPI);
}


// ** Define the total energy
float GetLightEnergy(float density, float probRain, float henyeyGreensteinFactor){
    float beer_laws = exp( -density * probRain);
    float powdered_sugar = 1.0 - exp( -2.0 * density);

    //float henyeyGreensteinFactor = HenyeyGreenstein(lightVector, rayDirection, g);
    float totalEnergy = 2.0 * beer_laws * powdered_sugar * henyeyGreensteinFactor;

    return totalEnergy;
}


// SampleCloudDensity
float SampleCloudDensity(vec3 samplepoint, vec3 weather_data, float relativeHeight, bool ischeap){
    // Add movement to textures
    vec3 wind_direction = vec3(1.0, 0.0, 0.0);
    float cloud_speed = 10.0;

    float cloud_top_offset = 5.0;

    // Skew in wind direction

    samplepoint += relativeHeight * wind_direction * cloud_top_offset * 0.005;
    samplepoint += (wind_direction + vec3(0.0, 1.0, 0.0))* float(time) * cloud_speed * 0.001;


    // Init Low Frequency Sampling ...
    vec4 low_frequency_noises = SampleLowFrequencyTexture(samplepoint);

    // Perly Worley noise:
    float low_freq_FBM = low_frequency_noises.g * 0.625 +
                         low_frequency_noises.b * 0.250 +
                         low_frequency_noises.a * 0.125;


    //low_freq_FBM = clamp(low_freq_FBM, 0.0, 1.0);


    // Remap with worley noise
    float base_cloud = Remap(low_frequency_noises.r, -(1.0 - low_freq_FBM), 1.0, 0.0, 1.0);
    //base_cloud = clamp(base_cloud, 0.0, 1.0);
    //Get the gradient density
    float density_height_gradient = GetDensityHeightGradientForPoint(samplepoint, weather_data, relativeHeight);

    // Apply the height function to base cloud
    // Until here the shape of cloud is defined!
    base_cloud = base_cloud * density_height_gradient;

    // Apply the coverage of data
    float cloud_coverage = weather_data.r;
    //cloud_coverage = clamp(cloud_coverage, 0.0, base_cloud)
    //base_cloud = clamp(base_cloud, cloud_coverage, 1.0);
    float base_cloud_with_coverage = Remap(base_cloud, cloud_coverage, 1.0, 0.0, 1.0);
    base_cloud_with_coverage = clamp(base_cloud_with_coverage, 0.0, 1.0);
    // Get more aestheticcal cloud
    base_cloud_with_coverage *= cloud_coverage*4.0;

    //base_cloud_with_coverage = clamp(base_cloud_with_coverage, 0.0, 1.0);

    float final_cloud = base_cloud_with_coverage;

    if(!ischeap && base_cloud_with_coverage > 0.0){
        // sample high frequency texture
        vec3 curl_noise = SampleCurlNoiseTexture(samplepoint.xy);
        samplepoint.xy = samplepoint.xy + curl_noise.xy * (1.0 - relativeHeight);
        vec3 high_frequency_noises = SampleHighFrequencyTexture(samplepoint * 0.1);

        float high_freq_FBM =     (high_frequency_noises.r * 0.625 )
                                + (high_frequency_noises.g * 0.250 )
                                + (high_frequency_noises.b * 0.125 );

        // TODO: Paper propose other way to compute the height_fraction
        float high_freq_noise_modifier = mix(high_freq_FBM, 1.0 - high_freq_FBM, clamp(relativeHeight * 10.0, 0.0, 1.0));
        //high_freq_noise_modifier *= 0.35 * exp(-cloud_coverage * 0.75);
        //base_cloud_with_coverage = clamp(base_cloud_with_coverage, 0.2*high_freq_noise_modifier, 1.0);
        //high_freq_noise_modifier = clamp(high_freq_noise_modifier, 0.0, 1.0);
        base_cloud_with_coverage = clamp(base_cloud_with_coverage, 0.2 * high_freq_noise_modifier, 1.0);
        // Erode by remapping:
        final_cloud = Remap(base_cloud_with_coverage, 0.2 * high_freq_noise_modifier, 1.0, 0.0, 1.0);
        //final_cloud = clamp(final_cloud, 0.0, 1.0);
    }

    return final_cloud;
    //return clamp(base_cloud, 0.0, 1.0);
    //return base_cloud_with_coverage;
}

/*
 * @brief: SampleCloudAlongCone
 *
 */
float SampleCloudDensityAlongCone(vec3 posInAtm, vec3 rayOrigin, vec3 earthCenter, float stepsize, vec3 noise_kernel[6]){
    float density_along_cone = 0.0;

    float cone_spread_multiplier = stepsize;
    // Lighting ray-march loop
    for(int i = 0; i < 6; i++){
        vec3 lightpos = posInAtm + (cone_spread_multiplier * noise_kernel[i] * float(i));
        vec2 weatherpoint = GetRelativePointToWeatherMap(lightpos, rayOrigin, earthCenter, ATMOSPHERE_OUTER_RADIUS);
        vec3 weather_data = SampleWeatherTexture(weatherpoint);

        float relativeHeight = GetRelativeHeightInAtmosphere(lightpos, earthCenter);

        vec3 p = GetPositionInAtmosphere(lightpos, earthCenter, THICK_ATMOSPHERE)*0.5;
        // Sample cloud density:
        density_along_cone += SampleCloudDensity(p, weather_data, relativeHeight, false);
    }

    return density_along_cone;
}

//* Test Jitter options_
vec2 getJitterOffset (int index, vec2 dim)
{
    //index is a value from 0-15
    //Use pre generated halton sequence to jitter point --> halton sequence is a low discrepancy sampling pattern
    vec2 jitter = vec2(0.0);
    //index = index/2;
    jitter.x = HaltonSequence.x;
    jitter.y = HaltonSequence.y;
    return jitter/dim;
}
//* Test Ambient color
float calcSunIntensity()
{
        float zenithAngleCos = clamp(normalize(SunLocation).y, -1.0, 1.0);
        return 1000.0 * max(0.0, 1.0 - pow(2.71828, -((1.6110731557 - acos(zenithAngleCos)) / 1.5)));
}
vec3 calcSkyBetaR()
{
    vec3 rayleigh_total = vec3(5.804542996261093e-6, 1.3562911419845635e-5, 3.0265902468824876e-5);
        float rayleigh = 2.0;
        float sunFade = 1.0 - clamp(1.0 - exp(SunLocation.y / 450000.0), 0.0, 1.0);
        return vec3(rayleigh_total * (rayleigh - 1.0 + sunFade));
}

vec3 calcSkyBetaV()
{
    vec3 mie_const = vec3( 1.839991851443397, 2.779802391966052, 4.079047954386109);
        float turbidity = 10.0;
        float mie = 0.005;
    float c = (0.2 * turbidity) * 10e-18;
    return vec3(0.434 * c * mie_const * mie);
}
float rayleighPhase(float cosTheta)
{
    return 3.0 * (1.0 + cosTheta * cosTheta)/(16.0*MPI);
}
vec3 getAtmosphereColorPhysical(vec3 dir, vec3 sunDir, float sunIntensity)
{
    vec3 localcolor = vec3(0);

    sunDir = normalize(sunDir);
    float sunE = sunIntensity * calcSunIntensity();
    vec3 BetaR = calcSkyBetaR();
    vec3 BetaM = calcSkyBetaV();

    // optical length
    float zenith = acos(max(0.0, dir.y)); // acos?
    float inverse = 1.0 / (cos(zenith) + 0.15 * pow(93.885 - ((zenith * 180.0) / MPI), -1.253));
    float sR = 8.4e3 * inverse;
    float sM = 1.25e3 * inverse;

    vec3 fex = exp( -BetaR * sR + BetaM * sM);

    float cosTheta = dot(sunDir, dir);

    float rPhase = rayleighPhase(cosTheta * 0.5 + 0.5);
    vec3 betaRTheta = BetaR * rPhase;
    float mie_directional = 0.8;
    float mPhase = HenyeyGreenstein(dir, sunDir, mie_directional);
    vec3 betaMTheta = BetaM * mPhase;

    float yDot = 1.0 - sunDir.y;
    yDot *= yDot * yDot * yDot * yDot;
    vec3 betas = (betaRTheta + betaMTheta) / (BetaR + BetaM);
    vec3 Lin = pow(sunE * (betas) * (1.0 - fex), vec3(1.5));
    Lin *= mix(vec3(1), pow(sunE * (betas) * fex, vec3(0.5)), clamp(yDot, 0.0, 1.0));

    vec3 L0 = 0.1 * fex;
    float sun_angular_cos = 0.999956676946448;
    float sunDisk = smoothstep(sun_angular_cos, sun_angular_cos + 0.00002, cosTheta);
    L0 += (sunE * 15000.0 * fex) * sunDisk;

    localcolor = (Lin + L0) * 0.04 + vec3(0.0, 0.0003, 0.00075);

    // return color in HDR space
    return localcolor;
}



//* End jitter options

// ** Ray marching algorithm

vec3 RayMarch(vec3 rayOrigin, vec3 startPoint, vec3 endPoint, vec3 rayDirection, vec3 earthCenter, inout float density_inout){
    vec3 colorpixel                 = vec3(0.0);
    float density                   = 0.0;
    float cloud_test                = 0.0;
    int zero_density_sample_count   = 0;
    int sample_cout                 = 128;
    //int sample_cout                 = 128 - 64 * int(dot(rayDirection, vec3(0.0, 1.0, 0.0)));
    float thick_                    = length(endPoint - startPoint);
    float stepsize                  = float(thick_/float(sample_cout));
    float start_                    = length(startPoint - rayOrigin);
    float end_                      = length(endPoint   - rayOrigin);
    if(end_ < start_){
        float tmp = start_;
        start_ = end_;
        end_ = tmp;
    }
    vec3 stepSampling               = rayDirection/float(sample_cout);

    // Henyey Greenstein factor in light Sampling
    // Ray Light direction
    vec3 testsize                   = rayDirection/float(sample_cout);

    vec3 lightDirection             = normalize(SunLocation - rayOrigin);
    // g: eccentricity 0.2, proposed by paper
    float henyeyGreensteinFactor    = HenyeyGreenstein(lightDirection, rayDirection, 0.6);
    // Get the noise Kernell size 6
    vec3 noise_kernel[6];

    vec3 maxCompUnitVector;
    if(abs(lightDirection.x) > abs(lightDirection.y) && (abs(lightDirection.x) > abs(lightDirection.z))){
        maxCompUnitVector = vec3( abs(lightDirection.x), 0.0, 0.0);
    }
    else if(abs(lightDirection.y) > abs(lightDirection.x) && (abs(lightDirection.y) > abs(lightDirection.z))){
        maxCompUnitVector = vec3(0.0, abs(lightDirection.y), 0.0);
    }
    else{
        maxCompUnitVector = vec3(0.0, 0.0, abs(lightDirection.z));
    }
    vec3 zcom = cross(lightDirection, maxCompUnitVector);
    vec3 xcom = cross(zcom, lightDirection);
    mat3 sunRotMatrix = mat3(xcom, lightDirection, zcom);
    // Noise light kernell
    noise_kernel[0] = sunRotMatrix * vec3(0.1,    0.25, -0.15);
    noise_kernel[1] = sunRotMatrix * vec3(0.2,  0.50,  0.20);
    noise_kernel[2] = sunRotMatrix * vec3(-0.2,   0.10, -0.10);
    noise_kernel[3] = sunRotMatrix * vec3(-0.05,  0.75,  0.05);
    noise_kernel[4] = sunRotMatrix * vec3(-0.1,   1.00,  0.00);
    noise_kernel[5] = sunRotMatrix * vec3(0.0,    3.00,  0.00);

    vec3 samplepoint = vec3(0.5, 0.5, 0.0);
    // Start the raymarching loop
    //vec3 samplepoint = GetPositionInAtmosphere(posInAtm, earthCenter, thick_);
    for(float t = start_; t < end_; t += stepsize){
        vec2 jitterLoc = getJitterOffset(0, vec2(80.0));
        vec3 posInAtm = rayOrigin + t * (rayDirection + vec3(jitterLoc.x, (jitterLoc.x + jitterLoc.y)*4.0, jitterLoc.y));
        // Sample the cloud data
        //vec3 samplepoint            = GetPositionInAtmosphere(posInAtm, earthCenter, thick_);
        vec2 weatherpoint           = GetRelativePointToWeatherMap(posInAtm, rayOrigin, earthCenter, ATMOSPHERE_OUTER_RADIUS);
        vec3 weather_data           = SampleWeatherTexture(weatherpoint);
        float relativeHeight        = GetRelativeHeightInAtmosphere(posInAtm, earthCenter);
        //float relativeHeight = GetHeightInAtmosphere(posInAtm, earthCenter, startPoint, rayDirection, rayOrigin, thick_);
        // Start with light test
        if(cloud_test > 0.0){
            samplepoint += testsize;
            samplepoint /=8.0;
            //samplepoint = KeepInBox(samplepoint);
            float sampled_density = SampleCloudDensity(samplepoint, weather_data, relativeHeight, false);
            if(sampled_density == 0.0)
                zero_density_sample_count++;
            if(zero_density_sample_count != 6){
                density += sampled_density * 0.07;

                // Start the light sampling
                if(sampled_density != 0.0){
                    // Make the Light Sampling
                    float precipitation;
                    if(weather_data.z > 0.9) precipitation = 5.0;
                    else if(weather_data.z < 0.1) precipitation = 0.05;
                    else precipitation = 2.0;

                    float density_along_light_ray = SampleCloudDensityAlongCone(posInAtm, rayOrigin, earthCenter, stepsize, noise_kernel);
                    float totalEnergy = GetLightEnergy(density_along_light_ray, precipitation, henyeyGreensteinFactor);
                    float transmitance = 1.0;
                    transmitance = mix(density, totalEnergy, (1.0 - density_along_light_ray));
                    //transmitance = clamp(transmitance, 0.0, 1.0);
                    colorpixel += vec3(transmitance * 0.4);
                    //colorpixel += vec3(sampled_density);
                    //colorpixel += vec3(totalEnergy * 1.0);
                }
                //samplepoint += stepSampling;
                //t += stepsize;
                //posInAtm = rayOrigin +  t * rayDirection;
                //colorpixel += vec3(sampled_density * 0.1);
                //if(sampled_density < 0){
                //    colorpixel = vec3(0.0);
                //    break;
                //}
            }
            else{
                cloud_test = 0.0;
                zero_density_sample_count = 0;
            }
        }
        //Light sampling
        else{
            cloud_test = SampleCloudDensity(samplepoint, weather_data, relativeHeight, true);
            if(cloud_test == 0.0){
                //samplepoint += stepSampling;
                //posInAtm += t * rayDirection;
                //t -= stepsize;
            }
        }

        if(density >= 1.0){
            density = 1.0;
            break;
        }
        // Define the position in atmosphere
        //samplepoint = KeepInBox(samplepoint);
    }
    density_inout = density;
    return colorpixel;
}


void main(){
    //float aspecRatio    = screenWidth / screenHeight;
    vec3  rayOrigin     = camera;
    //float x             = aspecRatio * (2.0 * gl_FragCoord.x/screenWidth - 1.0);
    //float y             = 2.0 * gl_FragCoord.y / screenHeight - 1.0;

    // Get ray Direction

    vec3 rayDirection       = normalize(pointPosition-camera);//GetRayDirection(cameraFront, cameraRight, cameraUp, x, y);

    //vec3 earthCenter        = vec3(rayOrigin.x, rayOrigin.y - EARTH_RADIUS, rayOrigin.z);
    vec3 earthCenter        = EarthCenter;
    vec3 innerIntersection  = GetIntersectionSphereRay(rayOrigin, rayDirection, earthCenter, ATMOSPHERE_INNER_RADIUS);
    vec3 outerIntersection  = GetIntersectionSphereRay(rayOrigin, rayDirection, earthCenter, ATMOSPHERE_OUTER_RADIUS);


    if(dot(vec3(0.0, 1.0, 0.0), rayDirection) < 0.0){
        vec3 colorNearHorizon = vec3(0.54, 0.23, 0.046) * 0.4;
        gl_FragColor =  vec4(colorNearHorizon, 1.0);
        return;
    }
    else if(dot(vec3(0.0, 1.0, 0.0), rayDirection) < 0.06){
        vec3 skycolor = getAtmosphereColorPhysical(rayDirection, SunLocation - rayOrigin, SunIntensity);
        skycolor *= 0.620;
        //vec3 colorNearHorizon = vec3(0.1, 0.1, 0.1);
        //color = vec4(colorNearHorizon, 1.0);
        gl_FragColor = vec4(skycolor, 1.0);
        return;
    }
    vec3 skycolor = getAtmosphereColorPhysical(rayDirection, SunLocation - rayOrigin, SunIntensity);
    skycolor *= 0.62;
    //vec3 skycolor = vec3(0.054687, 0.3, 0.57);
    // ** Perform the color of Sun
    vec3 sunLightDirection = normalize(SunLocation - rayOrigin);
    // ** Dot product to get the intensity of sun at some direction
    float sunFactorEnergy = dot(sunLightDirection, rayDirection);
    sunFactorEnergy = exp(2000.0 * (sunFactorEnergy - 1.0));
    vec3 sunColor = vec3(0.9608, 0.9529, 0.9137);

    //skycolor = mix(skycolor, 2.0 * sunColor, sunFactorEnergy);


    float density = 0.0;

    vec3 col = RayMarch(rayOrigin, innerIntersection, outerIntersection, rayDirection, earthCenter, density);
    col *= 0.04;
    density *= smoothstep(0.0, 1.0, min(1.0, Remap(rayDirection.y, 0.06, 0.4, 0.0, 1.0)));
    //vec3 sk_c = mix(col,  10.0 * sunColor, sunFactorEnergy);
    vec3 col_sky_ = mix(skycolor, col, density);

    //vec3 col_sky_ = mix(skycolor * 1.5, col, density);

    //col.x = Remap(skycolor.x, 0.0, col.x, 0.0, 1.0);
    //col.y = Remap(skycolor.y, 0.0, col.y, 0.0, 1.0);
    //col.z = Remap(skycolor.z, 0.0, col.z, 0.0, 1.0);
    //col = skycolor + col;
    //
    //col.x = clamp(col.x, 0.0, 1.0);
    //col.y = clamp(col.y, 0.0, 1.0);
    //col.z = clamp(col.z, 0.0, 1.0);

    //vec4 cc = SampleLowFrequencyTexture(vec3(vec2(x,y), 0.0));
    //color = vec4(cc.w, cc.w, cc.w, 1.0);
    //vec3 col = SampleCurlNoiseTexture(vec2(x, y)*0.5 + 0.5);
    //vec3 col = vec3(density, density, density);
    //color = vec4(col.xy, 0.0, 1.0);

    // Debug WeatherMap
    //vec2 wp = GetRelativePointToWeatherMap(outerIntersection, rayOrigin, earthCenter, ATMOSPHERE_OUTER_RADIUS);


    //vec3 c = SampleWeatherTexture(wp);
    //vec3 c = SampleWeatherTexture(vec2(x, y) * 0.5 + 0.5);
    //color = vec4(c, 1.0);
    gl_FragColor = vec4(col_sky_, 1.0);
}
