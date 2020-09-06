varying vec2 position;
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

void main(){
    vec4 color = texture2D( tex , position);
    vec3 pos = texture2D( posMap , position).rgb;
    float length = length(pos - lightPos);
    if(length<lightRange){
        color += texture2D( materialMap , position)*max(1.0-(length/lightRange) , 0.0);
    }
    gl_FragColor = color;
}
