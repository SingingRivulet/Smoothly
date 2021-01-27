/*
 * Copyright (c) 2013, elvman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY elvman ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL elvman BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
const float LOG2 = 1.442695;

uniform vec3        CameraPosition;  // Position of main position
uniform float        WaveHeight;

uniform vec4        WaterColor;
uniform float        ColorBlendFactor;

uniform sampler2D    WaterBump; //coverage
uniform sampler2D    RefractionMap; //coverage
uniform sampler2D    ReflectionMap; //coverage

uniform bool        FogEnabled;
uniform int            FogMode;
uniform vec2 lastPos;
uniform vec2 nowPos;

varying vec2 bumpMapTexCoord;
varying vec3 refractionMapTexCoord;
varying vec3 reflectionMapTexCoord;
varying vec3 position3D;
varying vec4 pointPosition;

vec2 getPosNormal(vec2 waveGlobal){
    vec2 waveTexPos = waveGlobal - nowPos + vec2(1024.0,1024.0);

    float h1 = texture2D(WaterBump , vec2(waveTexPos.x/2048.0 , waveTexPos.y/2048.0)).r;
    float h2 = texture2D(WaterBump , vec2((waveTexPos.x+1.0)/2048.0 , waveTexPos.y/2048.0)).r;
    float h3 = texture2D(WaterBump , vec2(waveTexPos.x/2048.0 , (waveTexPos.y+1.0)/2048.0)).r;
    vec3 p1 = vec3(0.0,h1,0.0);
    vec3 p2 = vec3(1.0,h2,0.0);
    vec3 p3 = vec3(0.0,h3,1.0);
    return normalize(cross(p3-p1,p2-p1)).xz;
}

vec2 smoothstep2d(vec2 a,vec2 b,float k){
    return vec2(
                mix(a.x,b.x,k),
                mix(a.y,b.y,k));
}

void main(){
    //bump color
    vec3 pointPosition3D = pointPosition.xyz/pointPosition.w;
    vec2 waveGlobal = vec2(floor(pointPosition3D.x),floor(pointPosition3D.z));

    vec2 bumpColor00 = getPosNormal(waveGlobal);
    vec2 bumpColor01 = getPosNormal(waveGlobal+vec2(0.0,1.0));
    vec2 bumpColor10 = getPosNormal(waveGlobal+vec2(1.0,0.0));
    vec2 bumpColor11 = getPosNormal(waveGlobal+vec2(1.0,1.0));

    vec2 bumpColor0 = smoothstep2d(bumpColor00,bumpColor01 , pointPosition3D.z-waveGlobal.y);
    vec2 bumpColor1 = smoothstep2d(bumpColor10,bumpColor11 , pointPosition3D.z-waveGlobal.y);

    vec2 bumpColor = smoothstep2d(bumpColor0,bumpColor1 , pointPosition3D.x-waveGlobal.x);

    vec2 perturbation = WaveHeight * bumpColor;
    
    //refraction
    vec2 ProjectedRefractionTexCoords = clamp(refractionMapTexCoord.xy / refractionMapTexCoord.z + perturbation, 0.0, 1.0);
    //calculate final refraction color
    vec4 refractiveColor = texture2D(RefractionMap, ProjectedRefractionTexCoords );
    
    //reflection
    vec2 ProjectedReflectionTexCoords = clamp(reflectionMapTexCoord.xy / reflectionMapTexCoord.z + perturbation, 0.0, 1.0);
    //calculate final reflection color
    vec4 reflectiveColor = texture2D(ReflectionMap, ProjectedReflectionTexCoords );

    //fresnel
    vec3 eyeVector = normalize(CameraPosition - position3D);
    vec3 upVector = vec3(0.0, 1.0, 0.0);
    
    //fresnel can not be lower than 0
    float fresnelTerm = clamp( 0.3+dot(eyeVector, upVector), 0.0 , 1.0);
    
    float fogFactor = 1.0;
    
    if (FogEnabled)
    {
        float z = gl_FragCoord.z / gl_FragCoord.w;

        if (FogMode == 1) //exp
        {
            float fogFactor = exp2(-gl_Fog.density * z * LOG2);
            fogFactor = clamp(fogFactor, 0.0, 1.0);
        }
        else if (FogMode == 0) //linear
        {
            fogFactor = (gl_Fog.end - z) / (gl_Fog.end - gl_Fog.start);
        }
        else if (FogMode == 2) //exp2
        {
            float fogFactor = exp2(-gl_Fog.density * gl_Fog.density * z * z * LOG2);
            fogFactor = clamp(fogFactor, 0.0, 1.0);
        }
    }
    
    vec4 combinedColor = refractiveColor * fresnelTerm + reflectiveColor * (1.0 - fresnelTerm);
    
    vec4 finalColor = ColorBlendFactor * WaterColor + (1.0 - ColorBlendFactor) * combinedColor;
    
    gl_FragData[0] = mix(gl_Fog.color, finalColor, clamp(fogFactor,0.0,1.0) );
    gl_FragData[1] = vec4(0.5,0.5,0.5,1.0);
    gl_FragData[2] = vec4(vec3(0.5,1.0,0.5),1.0);
    gl_FragData[3] = vec4(pointPosition.xyz/pointPosition.w,1.0);
}

