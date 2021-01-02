uniform sampler2D tex;

varying vec4 lcolor;
varying vec4 pointPosition;
varying float NdotL;
varying vec3 normal;
varying vec3 onormal;

void main(){
    
    vec2 t = gl_TexCoord[0].st;
    t = vec2(t.x,1.0-t.y);
    vec4 color = texture2D(tex,t);
    
    if(color.a<0.5)
        discard;

    vec3 lightcolor = lcolor.rgb;

    color.x*=lightcolor.x;
    color.y*=lightcolor.y;
    color.z*=lightcolor.z;
    
    gl_FragData[0] = vec4(gl_FragCoord.z,gl_FragCoord.z,gl_FragCoord.z,1);
    gl_FragData[1] = color;
    gl_FragData[2] = vec4(pointPosition.xyz/pointPosition.w,1.0);
}
