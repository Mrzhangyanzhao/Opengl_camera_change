//
//  main.cpp
//  OpenGL绘制金字塔
//
//  Created by yz on 2020/3/25.
//  Copyright © 2020 yz. All rights reserved.
//

/*
 无底金字塔。4个三角形。12个顶点构成
 1、4个三角形
 2、添加旋转
 3、添加棱线
 */


#include "GLTools.h"

//矩阵工具类(单元矩阵/矩阵/矩阵相乘/压栈/出栈/缩放/平移/旋转)
#include "GLMatrixStack.h"

//矩阵工具类(表示位置(设置vOrigin,vForward,vUp))
#include "GLFrame.h"

//矩阵工具类(快速设置正/透视投影矩阵，完成坐标从3D到2D的映射过程)
#include "GLFrustum.h"

//三角形批次类帮助类(传输顶点/光照/纹理/颜色数据到存储着色器中)
#include "GLBatch.h"

//变化管道(在代码中快速传输视图矩阵/投影矩阵/视图投影变换矩阵)
#include "GLGeometryTransform.h"

//数学库
#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif
//在Windows和Linux上，我们使⽤用freeglut的静态库版本并且需要添加⼀一个宏数学库



//存储着色器管理类
GLShaderManager shaderManager;

//模型试图矩阵
GLMatrixStack modelViewMatrix;
//投影矩阵
GLMatrixStack projectionMatrix;

//观察者视图坐标
GLFrame cameraFrame;
//图像环绕时的视图坐标
GLFrame objectFrame;

//图元绘制时的投影方式
GLFrustum viewFrustum;

//几何变换管道
GLGeometryTransform transformPipeLine;

//容器类
GLBatch triangleBatch;

GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//自定义函数(设置需要渲染图形的顶点/颜色数据等数据准备工作)
void SetupRC(){
    //初始化准备设置背景。初始化着色器 ，开启深度混合
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    shaderManager.InitializeStockShaders();
    glEnable(GL_DEPTH_TEST);
    
    //设置变换管道中模型视图矩阵/投影矩阵
    transformPipeLine.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    
    //设置观察者视图坐标位置
    //为了让效果明显，将观察者坐标位置Z移动往屏幕里移动15个单位位置
    // 参数：表示离屏幕之间的距离。 负数，是往屏幕后面移动；正数，往屏幕前面移动
    
    //************观察者动，物体不动**************//
    cameraFrame.MoveForward(-15.0f);
    
    //************观察者不动，物体动**************//
//    objectFrame.MoveForward(15.0f);
    
    //通过三角形创建金字塔
    GLfloat vPyramid[12][3]={
        -2.0f, 0.0f, -2.0f,
        2.0f, 0.0f, -2.0f,
        0.0f, 4.0f, 0.0f,

        2.0f, 0.0f, -2.0f,
        2.0f, 0.0f, 2.0f,
        0.0f, 4.0f, 0.0f,

        2.0f, 0.0f, 2.0f,
        -2.0f, 0.0f, 2.0f,
        0.0f, 4.0f, 0.0f,

        -2.0f, 0.0f, 2.0f,
        -2.0f, 0.0f, -2.0f,
        0.0f, 4.0f, 0.0f
    };
    
    //每三个顶点定义一个三角形、将顶点数据拷入、完成
    triangleBatch.Begin(GL_TRIANGLES, 12);
    triangleBatch.CopyVertexData3f(vPyramid);
    triangleBatch.End();
    
}

void DrawWireFramedBatch(GLBatch *pBatch){
    //填充图形内容
    /*绘制绿色部分*/
    /*GLShaderManager中的Uniform值--平面着色器
     *参数1 平面着色器
     *参数2 运行为几何图形指定一个4*4的变换矩阵，
     ——transformPipeline变换管线，指定了2个矩阵堆栈
     *参数3 颜色值
     */
    shaderManager.UseStockShader(GLT_SHADER_FLAT,transformPipeLine.GetModelViewProjectionMatrix(),vGreen);
    pBatch->Draw();
    
    /*绘制边框*/
    /*
     glEnable(GLenum mode) 用于启用各种功能，功能有参数决定
     glEnable()不能写在glBegin()和glEnd()中间
     
     GL_POLYGON_OFFSET_LINE  根据函数glPolygonOffset的设置，启用线的深度偏移
     GL_LINE_SMOOTH          执行后，过虑线点的锯齿
     GL_BLEND                启用颜色混合。例如实现半透明效果
     GL_DEPTH_TEST           启用深度测试 根据坐标的远近自动隐藏被遮住的图形（材料
     glDisable(GLenum mode); 用于关闭指定的功能 功能由参数决定
     */
    //偏移深度。在同一位置要绘制填充和边线，会产生z冲突。所以要偏移
    glPolygonOffset(-1.0f, -1.0f);
    glEnable(GL_POLYGON_OFFSET_LINE);
    
    // 画反锯齿，让黑边好看些
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //绘制线框几何黑色版 三种模式，实心，边框，点，可以作用在正面，背面，或者两面
    //通过调用glPolygonMode将多边形正面或者背面设为线框模式，实现线框渲染
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //设置线条宽度
    glLineWidth(2.5f);

    /* GLShaderManager 中的Uniform 值——平面着色器
     参数1：平面着色器
     参数2：运行为几何图形变换指定一个 4 * 4变换矩阵
         --transformPipeline.GetModelViewProjectionMatrix() 获取的
          GetMatrix函数就可以获得矩阵堆栈顶部的值
     参数3：颜色值（黑色）
     */

    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeLine.GetModelViewProjectionMatrix(), vBlack);
    pBatch->Draw();

    // 复原原本的设置
    ////通过调用glPolygonMode将多边形正面或者背面设为全部填充模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    
}

//渲染函数()
void RenderScene(void){
    //清理缓冲区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    //************观察者动，物体不动**************//
    //将观察者的坐标系压栈
    modelViewMatrix.PushMatrix();
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    //矩阵乘矩阵堆栈的顶部矩阵，相乘的结果随后存储在堆栈的顶部
    modelViewMatrix.MultMatrix(mCamera);

    //图形环绕坐标系压栈
    M3DMatrix44f mObjectFrame;
    //只要使用GetMutrix函数就可以获取矩阵堆栈顶部的值，这个函数可以进行2次重载。
    //用来使用GLShaderManager的使用。或者获取顶部矩阵的顶点副本数据。
    objectFrame.GetMatrix(mObjectFrame);
    //矩阵乘以矩阵堆栈的顶部矩阵。相乘的结果随后存储在堆栈的顶部
    modelViewMatrix.MultMatrix(mObjectFrame);

    /* GLShaderManager 中的Uniform 值——平面着色器
    参数1：平面着色器
    参数2：运行为几何图形变换指定一个 4 * 4变换矩阵
    --transformPipeline.GetModelViewProjectionMatrix() 获取的
    GetMatrix函数就可以获得矩阵堆栈顶部的值
    参数3：颜色值（黑色）
    */
    //************观察者动，物体不动**************//
    
    
    
    //************观察者不动，物体动**************//
//    modelViewMatrix.PushMatrix(objectFrame);
    //************观察者不动，物体动**************//
    
    //传递到存储着色器
    shaderManager.UseStockShader(GLT_SHADER_FLAT,transformPipeLine.GetModelViewProjectionMatrix(),vGreen);
    //提交着色器
    DrawWireFramedBatch(&triangleBatch);
    
    //还原到单位矩阵
    modelViewMatrix.PopMatrix();
    //后台缓冲 -> 交换
    glutSwapBuffers();
}

//窗口改变
void ChangeSize(int w,int h){
    glViewport(0, 0, w, h);
    //创建投影矩阵，并将它载入投影矩阵堆栈中
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    //调用顶部载入单元矩阵
    modelViewMatrix.LoadIdentity();
}

//特殊键位处理(上、下、左、右移动)
void SpecailKeys(int key,int x,int y){
    //围绕xyz固定轴旋转
    if (key == GLUT_KEY_UP) {
//        objectFrame.RotateLocalX(5.0);
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);
    }
    if (key==GLUT_KEY_DOWN){
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
    }
    if (key==GLUT_KEY_LEFT){
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
    }
    if (key==GLUT_KEY_RIGHT){
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
    }
    glutPostRedisplay();
}



int main (int argc,char * argv[]){
    
    //设置当前工作目录
    gltSetWorkingDirectory(argv[0]);
    //c 初始化GULT库
    glutInit(&argc, argv);
    //申请一个颜色缓存区、深度缓存区、双缓存区、模板缓存区
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800 , 600);
    glutCreateWindow("GL_TRIANGLES");
    
    //注册函数
    glutReshapeFunc(ChangeSize);
    //显示函数
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecailKeys);
    
    //确定初始化glew库，正常使用OpenGL框架
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        return 1;
    }
    
    //绘制
    SetupRC();
    glutMainLoop();
    
    return 0;
}
