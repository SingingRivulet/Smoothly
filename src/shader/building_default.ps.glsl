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

    color -= vec4(color.rgb,0.0)*shadow*0.3;

    gl_FragColor = color;
}
