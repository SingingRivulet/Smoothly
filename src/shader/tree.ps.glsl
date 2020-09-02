uniform mat4 shadowMatrix;
uniform mat4 modelMatrix;
uniform mat4 transformMatrix;
uniform float scan_animation_showing;
uniform float scan_animation_size;
uniform float shadowFactor;
uniform vec3 campos;
uniform sampler2D tex;
uniform sampler2D shadowMap;
uniform float clipY;
uniform int enableClipY;
uniform int clipYUp;

varying vec4 lcolor;
varying vec4 pointPosition;
varying vec3 normal;

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
    t = vec2(t.x,1.0-t.y);
    vec4 color = texture2D(tex,t);
    if(color.a<0.7)
        discard;
    gl_FragData[1] = color;
    color.x*=(lcolor.x+0.2);
    color.y*=(lcolor.y+0.2);
    color.z*=(lcolor.z+0.2);
    
    vec4 lightView4 = shadowMatrix * pointPosition;
    vec3 lightView = lightView4.xyz / lightView4.w;
    lightView = lightView * 0.5 + 0.5;
    
    float closestDepth;
    if(lightView.x<0.0 || lightView.x>1.0 || lightView.y<0.0 || lightView.y>1.0)
        closestDepth = 1.0;
    else
        closestDepth = texture2D(shadowMap, lightView.xy).r;
    float currentDepth = lightView.z;
    float shadow = currentDepth-0.001 < closestDepth  ? 0.0 : 1.0;

    color.x*=(lcolor.x+0.2);
    color.y*=(lcolor.y+0.2);
    color.z*=(lcolor.z+0.2);
    if(color.a<0.7)
        discard;

    color -= vec4(color.rgb,0.0)*shadow*shadowFactor;
    color.g += scan_animation_showing/exp(abs(length(pointPosition.xyz/pointPosition.w-campos)-scan_animation_size));
    gl_FragData[2] = vec4(normal*0.5 + vec3(0.5,0.5,0.5),1.0);
    
    gl_FragData[0] = color;
    gl_FragData[0].a =1.0;
}
