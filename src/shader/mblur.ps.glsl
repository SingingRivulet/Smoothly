varying vec2 position;
varying vec2 viewPos;
uniform sampler2D posMap;
uniform sampler2D resultMap;
uniform vec3 camera;
uniform float waterLevel;
uniform mat4 ProjMatrix;
uniform mat4 ViewMatrix;
uniform mat4 preViewMatrix;

uniform int windowWidth;
uniform int windowHeight;

uniform int mblurStep;

void main(){
    float size = 1.0/float(mblurStep);
    vec3 pos = texture2D(posMap,position).xyz;
    vec4 color = texture2D(resultMap,position);
    if(abs(pos.x)>1.0 && abs(pos.y)>1.0 && abs(pos.z)>1.0){
        vec4 prePos4 = ProjMatrix * preViewMatrix * vec4(pos,1.0);
        vec3 prePos = prePos4.xyz/prePos4.w;//归一化
        prePos = prePos * 0.5 + vec3(0.5);//变换到纹理空间
        vec2 deltaPos = prePos.xy - position;
        vec2 stepPos = deltaPos*size;
        for(int i=1;i<mblurStep;++i){
            color += texture2D(resultMap,(position+stepPos*float(i)));
        }
        color*=size;
    }
    gl_FragColor = color;
}
