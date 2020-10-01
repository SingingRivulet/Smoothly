uniform sampler2D tex;
uniform sampler2D shadowMap;
uniform mat4 shadowMatrix;
uniform mat4 modelMatrix;
uniform float shadowFactor;
uniform vec3 campos;
uniform float clipY;
uniform int enableClipY;
uniform int clipYUp;

uniform float scan_animation_showing;
uniform float scan_animation_size;

varying vec4 pointPosition;

void main(){
    vec4 tex_color = texture2D( tex, gl_TexCoord [ 0].xy);    
    gl_FragData[1] = vec4( tex_color.xyz, 1.0);   

    gl_FragData[0] = gl_FragData[1];
    gl_FragData[2] = vec4(vec3(0.5,0.5,0.5),1.0);
    gl_FragData[3] = vec4(pointPosition.xyz/pointPosition.w,1.0);
}
