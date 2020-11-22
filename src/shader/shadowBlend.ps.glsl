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
    vec3 normal = normalize((texture2D(normalMap,position).xyz*2.0)-vec3(1.0));//法线
    vec3 color = texture2D(tex,position).xyz;
    if(abs(pos.x)>0.1 || abs(pos.y)>0.1 || abs(pos.z)>0.1){
        float shadow = texture2D(shadowViewMap,position).x;
        color -= shadow*color*shadowFactor*clamp(dot(normal,normalize(lightDir)),0.0,1.0);
    }
    gl_FragColor = vec4(color,1.0);
}
