uniform sampler2D tex;
varying vec4 lcolor;
void main(){
    vec2 t = gl_TexCoord[0].st;
    t = vec2(t.x,1.0-t.y);
    vec4 color = texture2D(tex,t);
    color.x*=(lcolor.x+0.2);
    color.y*=(lcolor.y+0.2);
    color.z*=(lcolor.z+0.2);
    if(color.a<0.2)
        discard;
    gl_FragColor = color;
}
