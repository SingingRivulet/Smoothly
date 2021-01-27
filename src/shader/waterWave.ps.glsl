uniform sampler2D waveMap;
uniform vec2 lastPos;
uniform vec2 nowPos;
uniform vec2 waveMapSize;

varying vec2 position;

float getHeight(vec2 pos){
    if(pos.x>1.0 || pos.x<0.0 || pos.y>1.0 || pos.y<0.0)
        return 0.0;
    return texture2D(waveMap,pos).r;
}

void main(){
    vec2 dpos = position + (nowPos - lastPos)/2048.0;
    float x0 = getHeight(dpos);
    float x1 = getHeight(dpos+vec2(1.0/2048.0 , 0.0));
    float x2 = getHeight(dpos-vec2(1.0/2048.0 , 0.0));
    float x3 = getHeight(dpos+vec2(0.0 , 1.0/2048.0));
    float x4 = getHeight(dpos-vec2(0.0 , 1.0/2048.0));
    float waveValue = (x0+x1+x2+x3+x4) * 0.2;
    gl_FragColor = vec4(waveValue , 0.0 , 0.0 , 1.0);
}
