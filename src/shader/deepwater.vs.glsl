varying vec3 pointPosition;//坐标
varying vec3 wcolor;
void main(){
    gl_Position = ftransform();//设置坐标
    pointPosition = gl_Vertex.xyz; //把坐标点传给像素
    if(pointPosition.y>(-128.0))
        wcolor = vec3(0.1,0.1,0.2);
    else
        wcolor = vec3(0.0,0.0,0.0);
}
