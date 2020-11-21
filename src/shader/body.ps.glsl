uniform sampler2D tex;
uniform vec3 campos;
uniform float clipY;
uniform int enableClipY;
uniform int clipYUp;

uniform float scan_animation_showing;
uniform float scan_animation_size;

varying vec4 lcolor;
varying vec4 pointPosition;
varying float NdotL;
varying vec3 normal;
varying vec3 onormal;

void main(){
    if(enableClipY==1){
        float y = pointPosition.y/pointPosition.w;
        if(clipYUp==1){
            if(y>clipY)
                discard;
        }else{
            if(y<clipY)
                discard;
        }
    }
    
    vec2 t = gl_TexCoord[0].st;
    t = vec2(t.x,1.0-t.y);
    vec4 color = vec4(1.0,1.0,1.0,1.0);//texture2D(tex,t);
    gl_FragData[1] = color;

    vec3 lightcolor = lcolor.rgb;

    color.x*=(lightcolor.x+0.2);
    color.y*=(lightcolor.y+0.2);
    color.z*=(lightcolor.z+0.2);
    if(color.a<0.7)
        discard;

    color.rg += scan_animation_showing/exp(abs(length(pointPosition.xyz/pointPosition.w-campos)-scan_animation_size));

    gl_FragData[0] = color;
    gl_FragData[2] = vec4(onormal*0.5 + vec3(0.5,0.5,0.5),1.0);
    gl_FragData[3] = vec4(pointPosition.xyz/pointPosition.w,1.0);
}
