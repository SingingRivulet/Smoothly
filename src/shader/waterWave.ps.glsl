uniform sampler2D waveMap;
uniform float cellSize;
uniform vec2 deltaPos;
varying vec2 position;

float getHeight(vec2 pos){
    if(pos.x>1.0 || pos.x<0.0 || pos.y>1.0 || pos.y<0.0)
        return 0.0;
    return texture2D(waveMap,pos).r;
}
void main(){
    vec2 dpos = position+deltaPos;
    float x0 = getHeight(dpos);
    float x1 = getHeight(dpos+vec2(cellSize,0.0));
    float x2 = getHeight(dpos-vec2(cellSize,0.0));
    float x3 = getHeight(dpos+vec2(0.0,cellSize));
    float x4 = getHeight(dpos-vec2(0.0,cellSize));
    gl_FragColor = vec4((x1+x2+x3+x4)*0.5-x0 , 0.0 , 0.0 , 1.0);
}
