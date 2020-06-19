uniform sampler2D tex;
uniform sampler2D shadowMap;
uniform mat4 shadowMatrix;
uniform mat4 modelMatrix;

varying vec4 lcolor;
varying vec4 pointPosition;

void main(){
    vec2 t = gl_TexCoord[0].st;
    t = vec2(t.x,1.0-t.y);
    vec4 color = texture2D(tex,t);

    vec4 lightView4 = shadowMatrix * pointPosition;
    vec3 lightView = lightView4.xyz / lightView4.w;
    lightView = lightView * 0.5 + 0.5;

    float closestDepth = texture2D(shadowMap, lightView.xy).r;
    float currentDepth = lightView.z;
    float shadow = currentDepth+0.5 < closestDepth  ? 1.0 : 0.0;

    color.x*=(lcolor.x+0.2);
    color.y*=(lcolor.y+0.2);
    color.z*=(lcolor.z+0.2);
    if(color.a<0.7)
        discard;

    color -= color*shadow*0.5;

    gl_FragColor = color;
}
