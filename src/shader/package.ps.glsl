uniform mat4 modelMatrix;
uniform mat4 transformMatrix;
uniform float scan_animation_showing;
uniform float scan_animation_size;
uniform vec3 campos;
uniform sampler2D tex;
uniform float clipY;
uniform int enableClipY;
uniform int clipYUp;

varying vec4 lcolor;
varying vec4 pointPosition;
varying vec3 normal;
varying vec3 onormal;

void main(){
    if(enableClipY==1){
        float y = pointPosition.y;
        if(clipYUp==1){
            if(y>clipY)
                discard;
        }else{
            if(y<clipY)
                discard;
        }
    }
    vec2 t = gl_TexCoord[0].st;
    vec4 color = texture2D(tex,t);
    if(color.a<0.7)
        discard;
    gl_FragData[1] = color;
    color.x*=(lcolor.x+0.2);
    color.y*=(lcolor.y+0.2);
    color.z*=(lcolor.z+0.2);
    
    color.g += scan_animation_showing/exp(abs(length(pointPosition.xyz/pointPosition.w-campos)-scan_animation_size));
    gl_FragData[2] = vec4(onormal*0.5 + vec3(0.5,0.5,0.5),1.0);
    
    gl_FragData[0] = color;
    gl_FragData[0].a =1.0;
    gl_FragData[3] = vec4(pointPosition.xyz/pointPosition.w,1.0);
}
