varying vec2 position;
uniform sampler2D tex;
uniform sampler2D dep;
void main(){
    gl_FragColor = texture2D(tex,position);
}
