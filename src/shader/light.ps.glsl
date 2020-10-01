varying vec2 viewPos;
uniform sampler2D tex;
uniform sampler2D depth;
uniform sampler2D normalMap;
uniform sampler2D materialMap;
uniform sampler2D posMap;
uniform vec3 camera;
uniform float waterLevel;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform float lightRange;
uniform int windowWidth;
uniform int windowHeight;

void main(){
    vec2 position = vec2(gl_FragCoord.x/float(windowWidth),gl_FragCoord.y/float(windowHeight));
    vec4 color = texture2D( tex , position);
    vec3 pos = texture2D( posMap , position).rgb;
    if(abs(pos.x)<0.1 && abs(pos.y)<0.1 && abs(pos.z)<0.1){
        discard;
    }
    float length = length(pos - lightPos);
    if(length<lightRange){
        color += texture2D( materialMap , position)*max(1.0-(length/lightRange) , 0.0);
        gl_FragColor = color;
    }else{
        discard;
    }
}
