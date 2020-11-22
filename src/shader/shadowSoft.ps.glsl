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

void main(){
    vec3 pos = texture2D(posMap,position).xyz;//位置
    if(abs(pos.x)>0.1 || abs(pos.y)>0.1 || abs(pos.z)>0.1){
        float color = 0.0;
        int seg = 2;
        int i = -seg;
        int j = 0;
        float f = 0.0;
        float dvx = 2.0/float(windowWidth);
        float dvy = 2.0/float(windowHeight);
        float tot = 0.0;
        for(; i <= seg; ++i){
            for(j = -seg; j <= seg; ++j){
                f = (1.1 - sqrt(float(i*i + j*j))/8.0);
                f *= f;
                tot += f;
                color += texture2D( shadowViewMap, vec2(position.x + float(j) * dvx, position.y + float(i) * dvy) ).r * f;
            }
        }
        color /= tot;
        gl_FragColor = vec4(vec3(color),1.0);
    }else{
        gl_FragColor = vec4(vec3(0.0),1.0);
    }
}
