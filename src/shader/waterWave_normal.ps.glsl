uniform sampler2D waveMap;
varying vec2 position;

void main(){
    float delta = 1.0/2048.0;
    float h1 = texture2D(waveMap , vec2(position.x       , position.y)).r;
    float h2 = texture2D(waveMap , vec2(position.x+delta , position.y)).r;
    float h3 = texture2D(waveMap , vec2(position.x       , position.y+delta)).r;
    vec3 p1 = vec3(0.0,h1,0.0);
    vec3 p2 = vec3(1.0,h2,0.0);
    vec3 p3 = vec3(0.0,h3,1.0);
    gl_FragColor = vec4(normalize(cross(p3-p1,p2-p1)) , 1.0);
}
