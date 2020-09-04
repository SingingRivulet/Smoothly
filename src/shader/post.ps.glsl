varying vec2 position;
varying vec2 viewPos;
uniform sampler2D tex;
uniform sampler2D depth;
uniform sampler2D normalMap;
uniform vec3 camera;
uniform float waterLevel;

void main(){
    
    vec3 bloom = vec3(0.0);
    int seg = 5;
    int i = -seg;
    int j = 0;
    float f = 0.0;
    float dv = 2.0/512.0;
    float tot = 0.0;
    for(; i <= seg; ++i){
        for(j = -seg; j <= seg; ++j){
            f = (1.1 - sqrt(float(i*i + j*j))/8.0);
            f *= f;
            tot += f;
            vec3 ocol = texture2D( tex, vec2(position.x + float(j) * dv, position.y + float(i) * dv) ).rgb;
            ocol = vec3(max(ocol.r-0.7,0.0),max(ocol.g-0.7,0.0),max(ocol.b-0.7,0.0))*2.0;
            bloom += ocol * f;
        }
    }
    bloom /= tot;
    vec4 color = texture2D(tex,position);
    color.r+=bloom.r;
    color.g+=bloom.g;
    color.b+=bloom.b;
    
    //水下
    float dep = texture2D(depth,position).r;
    float wk = clamp((camera.y - waterLevel),0.2,1.0);
    color.r *= wk;
    color.g *= wk;
    
    gl_FragColor = color;
}
