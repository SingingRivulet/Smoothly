varying vec3 camera;
varying vec3 pointPosition;//坐标
varying vec4 screen;
void main(){
    camera = vec3(0.0,0.0,0.0);
    screen = ftransform();//设置坐标
    gl_Position = screen;
    pointPosition = gl_Vertex.xyz; //把坐标点传给像素
}
