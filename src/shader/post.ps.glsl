varying vec2 position;
uniform sampler2D tex;
uniform sampler2D depth;
uniform vec3 camera;
uniform float waterLevel;
void main(){
    vec4 color = texture2D(tex,position);
    float dep = texture2D(depth,position).r;
    float wk = clamp((camera.y - waterLevel),0.2,1.0);
    color.r *= wk;
    color.g *= wk;
    gl_FragColor = color;
}
