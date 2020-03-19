varying vec3 camera;
varying vec3 pointPosition;//坐标
varying vec4 screen;
void main(){
    camera = vec3(0.0,0.0,0.0);
    //screen = ftransform();//设置坐标

    if(gl_Color.g>0.5){//顶部
        gl_Position = vec4(gl_Vertex.xy,0.0,1.0);
        pointPosition = gl_Vertex.xzy; //把坐标点传给像素
    }else{
        if(gl_Color.r>0.5){
            if(gl_Color.b<0.5){
                //前
                gl_Position = vec4(gl_Vertex.xy,0.0,1.0);
                pointPosition = vec3(gl_Vertex.z,gl_Vertex.y,gl_Vertex.x);
            }else{
                //后
                gl_Position = vec4(gl_Vertex.xy,0.0,1.0);
                pointPosition = vec3(-gl_Vertex.z,gl_Vertex.y,-gl_Vertex.x);
            }
        }else{
            if(gl_Color.b<0.5){
                //左
                gl_Position = vec4(gl_Vertex.xy,0.0,1.0);
                pointPosition = vec3(gl_Vertex.x,gl_Vertex.y,-gl_Vertex.z);
            }else{
                //右
                gl_Position = vec4(gl_Vertex.xy,0.0,1.0);
                pointPosition = vec3(-gl_Vertex.x,gl_Vertex.y,gl_Vertex.z);
            }
        }
    }
}
