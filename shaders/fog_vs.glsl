uniform vec4 vViewPosition;       //摄像机的位置

varying float fogFactor;              //雾的权重


void main(void)
{
    gl_Position = ftransform();
    gl_TexCoord[0] = gl_MultiTexCoord0;
    
    const float LOG2 = 1.442695;
     float fogDensity = 0.001;              //这个参数代表了雾的浓度，越大雾越浓
    //gl_FogFragCoord = length(vViewPosition.xyz-gl_Vertex.xyz);   //计算视点与顶点的距离

    gl_FogFragCoord = abs(gl_Position.z);                                     //这样获得距离效率较高


    fogFactor = exp2( -fogDensity * fogDensity *                             //计算雾的权重
    gl_FogFragCoord * gl_FogFragCoord * LOG2 );
    fogFactor = clamp(fogFactor, 0.0, 1.0);                                     //越界处理
   
}
