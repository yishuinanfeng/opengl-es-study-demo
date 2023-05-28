#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <string.h>
#include <android/bitmap.h>
#include <unistd.h>
#include "FragmentShader.h"
#include "glm/glm/gtc/matrix_transform.hpp"
#include "glm/glm/ext.hpp"
#include "glm/glm/detail/_noise.hpp"
#include "BitmapInfo.h"
#include "Shader.h"
#include <vector>


using namespace glm;


#define BITMAP_INFO_LOGD(...) __android_log_print(ANDROID_LOG_WARN,"yuvOpenGlDemo",__VA_ARGS__)

enum enum_filter_type {
    //无滤镜效果
    filter_type_none,
    //灰度图
    filter_type_gray,
    //反色
    filter_type_oppo,
    //反色灰度
    filter_type_oppo_gray,
    //2分屏
    filter_type_divide_2,
    //4分屏
    filter_type_divide_4
};

#define SCALE_DURATION  600
#define SKIP_DURATION  100
#define MAX_DIFF_SCALE  2.0f


extern "C" JNIEXPORT jstring JNICALL
Java_com_example_openglstudydemo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}




float getTransformScale(int scaleDuration, int frame);




extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_drawPoints(JNIEnv *env, jobject thiz,
                                                      jobject surface) {

//1.获取原始窗口
    //be sure to use ANativeWindow_release()
    // * when done with it so that it doesn't leak.
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 4,
            EGL_GREEN_SIZE, 4,
            EGL_BLUE_SIZE, 4,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

//    BITMAP_INFO_LOGD("eglChooseConfig eglConfig:" + eglConfig);

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    Shader shader(vertexSimpleShape,fragSimpleShape);
    shader.use();


//    //加入三维顶点数据
//    static float rectangleVer[] = {
//            1.0f, -1.0f, 0.0f,
//            -1.0f, -1.0f, 0.0f,
//            1.0f, 1.0f, 0.0f,
//            -1.0f, 1.0f, 0.0f
//    };

//三个点坐标
    static float pointsVer[] = {
            0.8f, -0.8f, 0.0f,
            -0.8f, -0.8f, 0.0f,
            0.0f, 0.8f, 0.0f,
    };

//    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));旧的传输数据方式
//通过layout传输数据，传给了着色器中layout为0的变量
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, pointsVer);
    //打开layout为0的变量传输开关
    glEnableVertexAttribArray(0);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
//绘制三个点
    glDrawArrays(GL_POINTS, 0, 3);
    //窗口显示，交换双缓冲区
    eglSwapBuffers(display, winSurface);
}

float getTransformRatio(int scaleDuration, int frame) {
    int remainder = frame % scaleDuration;
    BITMAP_INFO_LOGD("ScaleFilter onDraw remainder:%d", remainder);
    float ratio;
    //放大的时候是线性变换，即放大系数和时间成正比。算出每个周期的帧数占一个周期的比例
//    if (remainder < scaleDuration / 2) {
//        ratio = remainder * 1.0F / scaleDuration;
//    } else {
    //加速度增大
//    ratio = static_cast<float>(pow(remainder * 1.0F / scaleDuration, 1.1));
    ratio = static_cast<float>(remainder * 1.0F / scaleDuration);
//    }

    BITMAP_INFO_LOGD("ratio:%f", ratio);

    //最大缩放倍数为1.5F
    float scale = MAX_DIFF_SCALE * ratio;
    //在增大到到达下一个缩放周期的时候ratio变为0，scale立刻变为1，就瞬间缩小为原本大小
    if (scale < 1) {
        scale = 1;
    }
    BITMAP_INFO_LOGD("scale:%f", scale);
    return scale;

}

float getTransformScale(int scaleDuration, int frame) {
    int remainder = frame % scaleDuration;
    BITMAP_INFO_LOGD("ScaleFilter onDraw remainder:%d", remainder);
    float ratio;
    //放大的时候是线性变换，即放大系数和时间成正比。算出每个周期的帧数占一个周期的比例
//    if (remainder < scaleDuration / 2) {
//        ratio = remainder * 1.0F / scaleDuration;
//    } else {
    //加速度增大
    ratio = static_cast<float>(pow(remainder * 1.0F / scaleDuration, 3));
//    }

    BITMAP_INFO_LOGD("ratio:%f", ratio);

    //最大缩放倍数为1.5F
    float scale = MAX_DIFF_SCALE * ratio;
    //在增大到到达下一个缩放周期的时候ratio变为0，scale立刻变为1，就瞬间缩小为原本大小
    if (scale < 1) {
        scale = 1;
    }
    BITMAP_INFO_LOGD("scale:%f", scale);
    return scale;

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_drawLine(JNIEnv *env, jobject thiz,
                                                    jobject surface) {

//1.获取原始窗口
    //be sure to use ANativeWindow_release()
    // * when done with it so that it doesn't leak.
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 4,
            EGL_GREEN_SIZE, 4,
            EGL_BLUE_SIZE, 4,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

//    BITMAP_INFO_LOGD("eglChooseConfig eglConfig:" + eglConfig);

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    Shader shader(vertexSimpleShape,fragSimpleShape);
    shader.use();

//    //加入三维顶点数据
//    static float rectangleVer[] = {
//            1.0f, -1.0f, 0.0f,
//            -1.0f, -1.0f, 0.0f,
//            1.0f, 1.0f, 0.0f,
//            -1.0f, 1.0f, 0.0f
//    };

//四个点坐标
    static float lineVer[] = {
            0.8f, -0.8f, 0.0f,
            -0.8f, -0.8f, 0.0f,
            0.0f, 0.8f, 0.0f,
            0.4f, 0.8f, 0.0f,
    };

//    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));旧的传输数据方式
//通过layout传输数据，传给了着色器中layout为0的变量
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, lineVer);
    //打开layout为0的变量传输开关
    glEnableVertexAttribArray(0);


    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    //设置线段宽度
    glLineWidth(20);
//绘制2条线段
    // glDrawArrays(GL_LINES, 0, 4);

    // 绘制2条线段
//     glDrawArrays(GL_LINE_STRIP, 0, 4);
//绘制2条线段
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    //窗口显示，交换双缓冲区
    eglSwapBuffers(display, winSurface);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_drawLineWithColor(JNIEnv *env, jobject thiz,
                                                             jobject surface) {

//1.获取原始窗口
    //be sure to use ANativeWindow_release()
    // * when done with it so that it doesn't leak.
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 4,
            EGL_GREEN_SIZE, 4,
            EGL_BLUE_SIZE, 4,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

//    BITMAP_INFO_LOGD("eglChooseConfig eglConfig:" + eglConfig);

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    Shader shader(vertexSimpleShapeWithColor,fragSimpleShape);
    shader.use();

//    //加入三维顶点数据
//    static float rectangleVer[] = {
//            1.0f, -1.0f, 0.0f,
//            -1.0f, -1.0f, 0.0f,
//            1.0f, 1.0f, 0.0f,
//            -1.0f, 1.0f, 0.0f
//    };

//四个点坐标
    static float lineVer[] = {
            0.8f, 0.0f, 0.0f,
            0.0, 0.0, 1.0,//颜色
            -0.8f, 0.0f, 0.0f,
            1.0, 0.0, 0.0,//颜色
    };

//    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));旧的传输数据方式
//通过layout传输数据，传给了着色器中layout为0的变量
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, lineVer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, lineVer + 3);
    //打开layout为0的变量传输开关
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);


    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    //设置线段宽度
    glLineWidth(20);
//绘制2条线段
    // glDrawArrays(GL_LINES, 0, 4);

    // 绘制2条线段
//     glDrawArrays(GL_LINE_STRIP, 0, 4);
//绘制2条线段
    glDrawArrays(GL_LINES, 0, 2);

    //窗口显示，交换双缓冲区
    eglSwapBuffers(display, winSurface);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_drawTriangle(JNIEnv *env, jobject thiz,
                                                        jobject surface) {

//1.获取原始窗口
    //be sure to use ANativeWindow_release()
    // * when done with it so that it doesn't leak.
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 4,
            EGL_GREEN_SIZE, 4,
            EGL_BLUE_SIZE, 4,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

//    BITMAP_INFO_LOGD("eglChooseConfig eglConfig:" + eglConfig);

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    Shader shader(vertexSimpleShape,fragSimpleShape);
    shader.use();

//    //加入三维顶点数据
//    static float rectangleVer[] = {
//            1.0f, -1.0f, 0.0f,
//            -1.0f, -1.0f, 0.0f,
//            1.0f, 1.0f, 0.0f,
//            -1.0f, 1.0f, 0.0f
//    };

//    static float triangleVer[] = {
//            0.8f, 0.8f, 0.0f,
//            0.0f, 0.8f, 0.0f,
//            0.4f, 0.4f, 0.0f,
//            -0.8f, 0.5f, 0.0f,
//            -0.4f, 0.8f, 0.0f,
//            -0.8f, 0.8f, 0.0f,
//    };

    static float triangleVer[] = {
            0.8f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.8f, 0.0f,
    };

//    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, triangleVer);
    glEnableVertexAttribArray(0);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    //窗口显示，交换双缓冲区
    eglSwapBuffers(display, winSurface);

    shader.release();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_drawTriangleUniform(JNIEnv *env, jobject thiz,
                                                               jobject surface) {

//1.获取原始窗口
    //be sure to use ANativeWindow_release()
    // * when done with it so that it doesn't leak.
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 4,
            EGL_GREEN_SIZE, 4,
            EGL_BLUE_SIZE, 4,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

//    BITMAP_INFO_LOGD("eglChooseConfig eglConfig:" + eglConfig);

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    Shader shader(vertexSimpleUniform,fragSimpleUniform);
    int program = shader.use();

    static float triangleVer[] = {
            0.8f, 0.0f, 0.5f,
            0.0f, 0.0f, 0.5f,
            0.0f, 0.8f, 0.5f,
    };

    static float color[] = {
            0.0f, 1.0f, 0.0f, 1.0f
    };

//    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, triangleVer);
    glEnableVertexAttribArray(0);

    int colorLocation = glGetUniformLocation(program, "uTextColor");
    glUniform4fv(colorLocation, 1, color);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    //窗口显示，交换双缓冲区
    eglSwapBuffers(display, winSurface);

    shader.release();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_drawTriangleWithColorPass(JNIEnv *env, jobject thiz,
                                                                     jobject surface) {

//1.获取原始窗口
    //be sure to use ANativeWindow_release()
    // * when done with it so that it doesn't leak.
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 4,
            EGL_GREEN_SIZE, 4,
            EGL_BLUE_SIZE, 4,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

//    BITMAP_INFO_LOGD("eglChooseConfig eglConfig:" + eglConfig);

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    Shader shader(vertexSimpleShapeWithColor,fragSimpleShape);
    int program = shader.use();

    static float triangleVerWithColor[] = {
            0.0f, 0.8f, 0.0f,//顶点
            1.0, 0.0, 0.0,//颜色
            -0.8f, -0.8f, 0.0f,//顶点
            0.0, 1.0, 0.0,//颜色
            0.8f, -0.8f, 0.0f,//顶点
            0.0, 0.0, 1.0,//颜色
    };

    static float triangleVer[] = {
            0.0f, 0.8f, 0.0f,//顶点
            -0.8f, -0.8f, 0.0f,//顶点
            0.8f, -0.8f, 0.0f,//顶点
    };

    static float colors[] = {
            1.0, 0.0, 0.0,//颜色
            0.0, 1.0, 0.0,//颜色
            0.0, 0.0, 1.0,//颜色
    };

//    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, triangleVerWithColor);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, triangleVerWithColor + 3);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, triangleVer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12, colors);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    //窗口显示，交换双缓冲区
    eglSwapBuffers(display, winSurface);
    //释放着色器程序对象
    shader.release();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_drawTriangleWithBufferObj(JNIEnv *env, jobject thiz,
                                                                     jobject surface) {

//1.获取原始窗口
    //be sure to use ANativeWindow_release()
    // * when done with it so that it doesn't leak.
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 4,
            EGL_GREEN_SIZE, 4,
            EGL_BLUE_SIZE, 4,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

//    BITMAP_INFO_LOGD("eglChooseConfig eglConfig:" + eglConfig);

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    Shader shader(vertexSimpleShapeWithColor,fragSimpleShapeWithColor);
    int program = shader.use();

    static float triangleVerWithColor[] = {
            0.0f, 0.8f, 0.0f,//顶点
            1.0, 0.0, 0.0,//颜色
            0.8f, 0.8f, 0.0f,//顶点
            0.0, 1.0, 0.0,//颜色
            0.0f, 0.0f, 0.0f,//顶点
            0.0, 0.0, 1.0,//颜色
    };

    static float triangleVerWithColor1[] = {
            0.0f, -0.8f, 0.0f,//顶点
            1.0, 0.0, 0.0,//颜色
            -0.8f, -0.8f, 0.0f,//顶点
            0.0, 1.0, 0.0,//颜色
            0.0f, 0.0f, 0.0f,//顶点
            0.0, 0.0, 1.0,//颜色
    };

//    static float triangleVer[] = {
//            0.0f, 0.8f, 0.0f,//顶点
//            -0.8f, -0.8f, 0.0f,//顶点
//            0.8f, -0.8f, 0.0f,//顶点
//    };
//
//    static float colors[] = {
//            1.0, 0.0, 0.0,//颜色
//            0.0, 1.0, 0.0,//颜色
//            0.0, 0.0, 1.0,//颜色
//    };

//    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));

    unsigned int VBOs[2];
    unsigned int VAOs[2];
    glGenVertexArrays(2, VAOs); // we can also generate multiple VAOs or buffers at the same time
    glGenBuffers(2, VBOs);

    glBindVertexArray(VAOs[0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVerWithColor), triangleVerWithColor,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void *) 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void *) (3 * 4));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVerWithColor1), triangleVerWithColor1,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void *) 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void *) (3 * 4));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(VAOs[0]);
//    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //窗口显示，交换双缓冲区
    eglSwapBuffers(display, winSurface);


    glDeleteBuffers(2, VBOs);
    glDeleteVertexArrays(2, VAOs);
    shader.release();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_drawTriangleWithEBO(JNIEnv *env, jobject thiz,
                                                               jobject surface) {

//1.获取原始窗口
    //be sure to use ANativeWindow_release()
    // * when done with it so that it doesn't leak.
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 4,
            EGL_GREEN_SIZE, 4,
            EGL_BLUE_SIZE, 4,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

//    BITMAP_INFO_LOGD("eglChooseConfig eglConfig:" + eglConfig);

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    Shader shader(vertexSimpleShapeWithColor,fragSimpleShapeEBO);
    int program = shader.use();

    float vertices[] = {
            0.5f, 0.5f, 0.0f,   // 右上角
            1.0, 0.0, 0.0,//颜色
            0.5f, -0.5f, 0.0f,  // 右下角
            0.0, 0.0, 1.0,//颜色
            -0.5f, -0.5f, 0.0f, // 左下角
            0.0, 1.0, 0.0,//颜色
            -0.5f, 0.5f, 0.0f,   // 左上角
            0.5, 0.5, 0.5,//颜色
    };

    unsigned int indices[] = {
            // 注意索引从0开始!
            // 此例的索引(0,1,2,3)就是顶点数组vertices的下标，
            // 这样可以由下标代表顶点组合成矩形
            0, 1, 3, // 第一个三角形
            1, 2, 3  // 第二个三角形
    };

    //  unsigned int VBO[1];
//    unsigned int VAO[1];
//    glGenVertexArrays(1, VAO); // we can also generate multiple VAO or buffers at the same time
//    glGenBuffers(1, VBO);

//    glBindVertexArray(VAO[0]);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVerWithColor), triangleVerWithColor, GL_STATIC_DRAW);


    unsigned int VBO;
    unsigned int VAO;
    glGenVertexArrays(1, &VAO); // we can also generate multiple VAO or buffers at the same time
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

//  复制我们的索引数组到一个索引缓冲中，供OpenGL使用
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);

//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void*)0);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void*)12);
//
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, vertices);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, vertices + 3);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);


    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

//    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *) 0);
    //窗口显示，交换双缓冲区
    eglSwapBuffers(display, winSurface);

//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //  glBindVertexArray(0);


    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    //释放着色器程序对象
    shader.release();

//    int k = 12;
//    BITMAP_INFO_LOGD("sizeof GL_UNSIGNED_INT:%d" ,sizeof(k) );
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_drawTwoTriangle(JNIEnv *env, jobject thiz,
                                                           jobject surface) {

//1.获取原始窗口
    //be sure to use ANativeWindow_release()
    // * when done with it so that it doesn't leak.
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES3_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    Shader shader(vertexSimpleShape,fragSimpleShape);
    int program = shader.use();

    //加入三维顶点数据
    static float rectangleVer[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, -0.25f, 0.0f,
            -1.0f, -0.25f, 0.0f
    };

    static float triangleVer[] = {
            0.8f, -0.8f, 0.0f,
            -0.8f, -0.8f, 0.0f,
            0.0f, 0.8f, 0.0f,
    };

    static float triangleVer1[] = {
            0.4f, -0.8f, 0.0f,
            -0.8f, -0.6f, 0.0f,
            0.0f, 0.9f, 0.0f,
    };

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));
    glEnableVertexAttribArray(apos);
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 0, triangleVer);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 0, rectangleVer);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //窗口显示，交换双缓冲区
    eglSwapBuffers(display, winSurface);

    shader.release();
}

#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>
#include <gtc/type_ptr.hpp>


extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_loadYuv(JNIEnv *env, jobject thiz,
                                                   jobject surface, jobject assetManager) {
    BITMAP_INFO_LOGD("loadYuv");


//    const char *url = env->GetStringUTFChars(jUrl, 0);
//
//    FILE *fp = fopen(url, "rb");
//    if (!fp) {
//        BITMAP_INFO_LOGD("oepn file %s fail", url);
//        return;
//    }
//    BITMAP_INFO_LOGD("open ulr is %s", url);
    //1.获取原始窗口
    //be sure to use ANativeWindow_release()
    // * when done with it so that it doesn't leak.
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    Shader shader(vertexShaderWithMatrix,fragYUV420P);
    int program = shader.use();

    //加入三维顶点数据
    static float ver[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f
    };

    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));
    glEnableVertexAttribArray(apos);
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 0, ver);

    //加入纹理坐标数据
    static float fragment[] = {
            1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
    };
    GLuint aTex = static_cast<GLuint>(glGetAttribLocation(program, "aTextCoord"));
    glEnableVertexAttribArray(aTex);
    glVertexAttribPointer(aTex, 2, GL_FLOAT, GL_FALSE, 0, fragment);

    int width = 640;
    int height = 272;

    //纹理初始化
    //设置纹理层对应的对应采样器？

    /**
     *  //获取一致变量的存储位置
    GLint textureUniformY = glGetUniformLocation(program, "SamplerY");
    GLint textureUniformU = glGetUniformLocation(program, "SamplerU");
    GLint textureUniformV = glGetUniformLocation(program, "SamplerV");
    //对几个纹理采样器变量进行设置
    glUniform1i(textureUniformY, 0);
    glUniform1i(textureUniformU, 1);
    glUniform1i(textureUniformV, 2);
     */
    //对sampler变量，使用函数glUniform1i和glUniform1iv进行设置
    glUniform1i(glGetUniformLocation(program, "yTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "uTexture"), 1);
    glUniform1i(glGetUniformLocation(program, "vTexture"), 2);
    //纹理ID
    GLuint textures[3] = {0};
    //创建若干个纹理对象，并且得到纹理ID
    glGenTextures(3, textures);

    //绑定纹理。后面的的设置和加载全部作用于当前绑定的纹理对象
    //GL_TEXTURE0、GL_TEXTURE1、GL_TEXTURE2 的就是纹理单元，GL_TEXTURE_1D、GL_TEXTURE_2D、CUBE_MAP为纹理目标
    //通过 glBindTexture 函数将纹理目标和纹理绑定后，对纹理目标所进行的操作都反映到对纹理上
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //放大的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    // 加载纹理到 OpenGL，读入 buffer 定义的位图数据，并把它复制到当前绑定的纹理对象
    // 当前绑定的纹理对象就会被附加上纹理图像。
    //width,height表示每几个像素公用一个yuv元素？比如width / 2表示横向每两个像素使用一个元素？
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本 默认0
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图（这里就是只取一个亮度的颜色通道的意思）
                 width,//加载的纹理宽度。最好为2的次幂(这里对y分量数据当做指定尺寸算，但显示尺寸会拉伸到全屏？)
                 height,//加载的纹理高度。最好为2的次幂
                 0,//纹理边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图
                 GL_UNSIGNED_BYTE,//像素点存储的数据类型
                 NULL //纹理的数据（先不传）
    );

    //绑定纹理
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本 默认0
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图（这里就是只取一个颜色通道的意思）
                 width / 2,//u数据数量为屏幕的4分之1
                 height / 2,
                 0,//边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图
                 GL_UNSIGNED_BYTE,//像素点存储的数据类型
                 NULL //纹理的数据（先不传）
    );

    //绑定纹理
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本 默认0
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图（这里就是只取一个颜色通道的意思）
                 width / 2,
                 height / 2,//v数据数量为屏幕的4分之1
                 0,//边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图
                 GL_UNSIGNED_BYTE,//像素点存储的数据类型
                 NULL //纹理的数据（先不传）
    );

    //列主序（column-major order）排列的。也就是说，矩阵的第一列元素在数组的前4个元素中，第二列元素在数组的第5到第8个元素中
    //缩小0.5
//    float arr[16] = {0.5, 0.0, 0.0,0.0,   0.0, 0.5, 0.0,0.0,    0.0, 0.0, 1.0,0.0,     0.0, 0.0, 0.0,1.0};
    //平移1.0
//    float arr[16] = {1.0, 0.0, 0.0,0.0,   0.0, 1.0, 0.0,0.0,    0.0, 0.0, 1.0,0.0,     1.0, 1.0, 0.0,1.0};
    //先缩小0.5，然后平移1.0
//    float arr[16] = {0.5, 0.0, 0.0,0.0,   0.0, 0.5, 0.0,0.0,    0.0, 0.0, 1.0,0.0,     1.0, 1.0, 0.0,1.0};
    //先平移1.0，然后缩小到0.5
//    float arr[16] = {0.5, 0.0, 0.0,0.0,   0.0, 0.5, 0.0,0.0,    0.0, 0.0, 1.0,0.0,    0.5, 0.5, 0.0,1.0};


//旋转theta
    float theta = 45 * M_PI / 180;
//    float arr[16] = {cos(theta), -sin(theta), 0.0, 0.0
//                     , sin(theta), cos(theta), 0.0, 0.0
//                     , 0.0, 0.0, 1.0, 0.0
//                     , 0.0, 0.0,0.0, 1.0};

    //先缩小到0.5倍，然后逆时针旋转45度，最后x,y方向分别平移0.5个单位
    float arr[16] = {0.5f * cos(theta), -0.5f * sin(theta), 0.0, 0.0, 0.5f * sin(theta),
                     0.5f * cos(theta), 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.5, 0.5, 0.0, 1.0};


    GLint uScaleMatrixLocation = glGetUniformLocation(program, "uMatrix");
    mat4 scaleMatrix = glm::mat4(1.0f);

    //使用手动创建的数组
//    glUniformMatrix4fv(uScaleMatrixLocation, 1, GL_FALSE, arr);


    //使用glm
    //注意：后面调用的先变换，和矩阵相乘一样，最右边的最先执行变换
    //x,y轴方向分别平移0.5
    scaleMatrix = glm::translate(scaleMatrix, vec3(0.5));
    //沿着（0，0，0）点逆时针旋转45度
    scaleMatrix = glm::rotate(scaleMatrix, glm::radians(45.0f), vec3(0.0f, 0.0f, 1.0f));
    //缩小到0.5倍
    scaleMatrix = glm::scale(scaleMatrix, vec3(0.5));
    //使用glm
    glUniformMatrix4fv(uScaleMatrixLocation, 1, GL_FALSE, glm::value_ptr(scaleMatrix));


    //创建3个buffer数组分别用于存放YUV三个分量
    unsigned char *buf[3] = {0};
    buf[0] = new unsigned char[width * height];//y
    buf[1] = new unsigned char[width * height / 4];//u
    buf[2] = new unsigned char[width * height / 4];//v

    //得到AAssetManager对象指针
    AAssetManager *mManeger = AAssetManager_fromJava(env, assetManager);
    //得到AAsset对象
    AAsset *dataAsset = AAssetManager_open(mManeger, "video1_640_272.yuv",
                                           AASSET_MODE_STREAMING);//get file read AAsset
    //文件总长度
    off_t dataBufferSize = AAsset_getLength(dataAsset);
    //纵帧数
    long frameCount = dataBufferSize / (width * height * 3 / 2);

    BITMAP_INFO_LOGD("frameCount:%d", frameCount);


    for (int i = 0; i < frameCount; ++i) {
        //读取y分量
        int bufYRead = AAsset_read(dataAsset, buf[0],
                                   width * height);  //begin to read data once time
        //读取u分量
        int bufURead = AAsset_read(dataAsset, buf[1],
                                   width * height / 4);  //begin to read data once time
        //读取v分量
        int bufVRead = AAsset_read(dataAsset, buf[2],
                                   width * height / 4);  //begin to read data once time
        BITMAP_INFO_LOGD("bufYRead:%d,bufURead:%d,bufVRead:%d", bufYRead, bufURead, bufVRead);

        //读到文件末尾了
        if (bufYRead <= 0 || bufURead <= 0 || bufVRead <= 0) {
            AAsset_close(dataAsset);
            return;
        }

        //  int c = dataRead(mManeger, "video1_640_272.yuv");

        //激活第一层纹理，绑定到创建的纹理
        //下面的width,height主要是显示尺寸？
        glActiveTexture(GL_TEXTURE0);
        //绑定y对应的纹理
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        //替换纹理，比重新使用glTexImage2D性能高多
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, 0,//相对原来的纹理的offset
                        width, height,//加载的纹理宽度、高度。最好为2的次幂
                        GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        buf[0]);

        //激活第二层纹理，绑定到创建的纹理
        glActiveTexture(GL_TEXTURE1);
        //绑定u对应的纹理
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        //替换纹理，比重新使用glTexImage2D性能高
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                        GL_UNSIGNED_BYTE,
                        buf[1]);

        //激活第三层纹理，绑定到创建的纹理
        glActiveTexture(GL_TEXTURE2);
        //绑定v对应的纹理
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        //替换纹理，比重新使用glTexImage2D性能高
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                        GL_UNSIGNED_BYTE,
                        buf[2]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //窗口显示，交换双缓冲区
        eglSwapBuffers(display, winSurface);

        //加一点延时效果避免帧率过快
        usleep(20000);
    }

    AAsset_close(dataAsset);


    shader.release();

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_drawTexture(JNIEnv *env, jobject thiz, jobject bitmap,
                                                       jobject bitmap1,
                                                       jobject surface) {

    //BITMAP_INFO_LOGD("drawTexture width:%d,height:%d", width, height);
    //1.获取原始窗口
    //be sure to use ANativeWindow_release()
    // * when done with it so that it doesn't leak.
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES3_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    Shader shader(vertexSimpleTexture,fragSimpleTexture);
    int program = shader.use();

    float vertices[] = {
            // positions         // texture coords
            0.8f, 0.4f, 0.0f, 1.0f, 1.0f, // top right
            0.8f, -0.4f, 0.0f, 1.0f, 0.0f, // bottom right
            -0.8f, -0.4f, 0.0f, 0.0f, 0.0f, // bottom left
            -0.8f, 0.4f, 0.0f, 0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    BITMAP_INFO_LOGD("glBufferData GL_ELEMENT_ARRAY_BUFFER");

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    BITMAP_INFO_LOGD("glEnableVertexAttribArray(1)");


    AndroidBitmapInfo bmpInfo;
    void *bmpPixels;

    if (AndroidBitmap_getInfo(env, bitmap, &bmpInfo) < 0) {
        BITMAP_INFO_LOGD("AndroidBitmap_getInfo() failed ! ");
        return;
    }

    AndroidBitmap_lockPixels(env, bitmap, &bmpPixels);

    BITMAP_INFO_LOGD("bitmap width:%d,height:%d", bmpInfo.width, bmpInfo.height);

    AndroidBitmapInfo bmpInfo1;
    void *bmpPixels1;

    if (AndroidBitmap_getInfo(env, bitmap1, &bmpInfo1) < 0) {
        BITMAP_INFO_LOGD("AndroidBitmap_getInfo() failed ! ");
        return;
    }

    AndroidBitmap_lockPixels(env, bitmap1, &bmpPixels1);

    BITMAP_INFO_LOGD("bitmap width:%d,height:%d", bmpInfo1.width, bmpInfo1.height);

    if (bmpPixels == nullptr || bmpPixels1 == nullptr) {
        return;
    }


    // load and create a texture
    // -------------------------
    unsigned int texture1, texture2;
    //-------------------- texture1的配置start ------------------------------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters（配置纹理环绕）
    //横坐标环绕配置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_REPEAT);    // set texture wrapping to GL_REPEAT (default wrapping method)
    //纵坐标环绕配置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters（配置纹理过滤）
    //纹理分辨率大于图元分辨率，即纹理需要被缩小的过滤配置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //纹理分辨率小于图元分辨率，即纹理需要被放大的过滤配置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmpInfo.width, bmpInfo.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, bmpPixels);
    AndroidBitmap_unlockPixels(env, bitmap);
    //-------------------- texture1的配置end ------------------------------


    //-------------------- texture2的配置start ------------------------------
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_REPEAT);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmpInfo1.width, bmpInfo1.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, bmpPixels1);
    AndroidBitmap_unlockPixels(env, bitmap1);

    //-------------------- texture2的配置end ------------------------------

    //对着色器中的纹理单元变量进行赋值
    glUniform1i(glGetUniformLocation(program, "ourTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "ourTexture1"), 1);

    //将纹理单元和纹理对象进行绑定
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
//    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    //    glDrawArrays(GL_TRIANGLES, 0, 3);


    //窗口显示，交换双缓冲区
    eglSwapBuffers(display, winSurface);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    //释放着色器程序对象
    shader.release();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_draw3DTexture(JNIEnv *env, jobject thiz, jobject bitmap,
                                                         jobject bitmap1, jobject surface,
                                                         jint screenWidth, jint screenHeight) {

    //BITMAP_INFO_LOGD("drawTexture width:%d,height:%d", width, height);
    //1.获取原始窗口
    //be sure to use ANativeWindow_release()
    // * when done with it so that it doesn't leak.
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES3_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    Shader shader(vertexShader3D,frag3DTexture);
    int program = shader.use();

    float vertices[] = {
            // positions         // texture coords
            0.5f, 0.5f, 0.0f, 1.0f, 1.0f, // top right
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // bottom right
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
            -0.5f, 0.5f, 0.0f, 0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    BITMAP_INFO_LOGD("glBufferData GL_ELEMENT_ARRAY_BUFFER");

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    BITMAP_INFO_LOGD("glEnableVertexAttribArray(1)");
    //模型矩阵，将局部坐标转换为世界坐标
    glm::mat4 model = glm::mat4(1.0f);
    //视图矩阵，确定物体和摄像机的相对位置
    glm::mat4 view = glm::mat4(1.0f);
    //透视投影矩阵，实现近大远小的效果
    glm::mat4 projection = glm::mat4(1.0f);
    //沿着x轴旋转
    model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // 注意，我们将矩阵向我们要进行移动场景的反方向移动。（右手坐标系，所以z正方形从屏幕指向外部）
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    BITMAP_INFO_LOGD("glm::perspective:%d,height:%d", screenWidth, screenHeight);
    projection = glm::perspective(glm::radians(45.0f), (float) screenWidth / (float) screenHeight,
                                  0.1f,
                                  100.0f);
    BITMAP_INFO_LOGD("mat4 init");
    GLint modelLoc = glGetUniformLocation(program, "model");
    GLint viewLoc = glGetUniformLocation(program, "view");
    GLint projectionLoc = glGetUniformLocation(program, "projection");
    BITMAP_INFO_LOGD("glGetUniformLocation");
    // pass them to the shaders (3 different ways)
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    BITMAP_INFO_LOGD("glUniformMatrix4fv");
    AndroidBitmapInfo bmpInfo;
    void *bmpPixels;

    if (AndroidBitmap_getInfo(env, bitmap, &bmpInfo) < 0) {
        BITMAP_INFO_LOGD("AndroidBitmap_getInfo() failed ! ");
        return;
    }

    AndroidBitmap_lockPixels(env, bitmap, &bmpPixels);

    BITMAP_INFO_LOGD("bitmap width:%d,height:%d", bmpInfo.width, bmpInfo.height);

    AndroidBitmapInfo bmpInfo1;
    void *bmpPixels1;

    if (AndroidBitmap_getInfo(env, bitmap1, &bmpInfo1) < 0) {
        BITMAP_INFO_LOGD("AndroidBitmap_getInfo() failed ! ");
        return;
    }

    AndroidBitmap_lockPixels(env, bitmap1, &bmpPixels1);

    BITMAP_INFO_LOGD("bitmap width:%d,height:%d", bmpInfo1.width, bmpInfo1.height);

    if (bmpPixels == nullptr || bmpPixels1 == nullptr) {
        return;
    }


    // load and create a texture
    // -------------------------
//    unsigned int texture1, texture2;
    unsigned int texture1;
    //-------------------- texture1的配置start ------------------------------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters（配置纹理环绕）
    //横坐标环绕配置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_REPEAT);    // set texture wrapping to GL_REPEAT (default wrapping method)
    //纵坐标环绕配置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters（配置纹理过滤）
    //纹理分辨率大于图元分辨率，即纹理需要被缩小的过滤配置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //纹理分辨率小于图元分辨率，即纹理需要被放大的过滤配置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmpInfo.width, bmpInfo.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, bmpPixels);
    AndroidBitmap_unlockPixels(env, bitmap);
    //-------------------- texture1的配置end ------------------------------


    //-------------------- texture2的配置start ------------------------------
//    glGenTextures(1, &texture2);
//    glBindTexture(GL_TEXTURE_2D, texture2);
//    // set the texture wrapping parameters
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
//                    GL_REPEAT);    // set texture wrapping to GL_REPEAT (default wrapping method)
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//    // set texture filtering parameters
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmpInfo1.width, bmpInfo1.height, 0, GL_RGBA,
//                 GL_UNSIGNED_BYTE, bmpPixels1);
//    AndroidBitmap_unlockPixels(env, bitmap1);

    //-------------------- texture2的配置end ------------------------------

    //对着色器中的纹理单元变量进行赋值
    glUniform1i(glGetUniformLocation(program, "ourTexture"), 0);
//    glUniform1i(glGetUniformLocation(program, "ourTexture1"), 1);

    //将纹理单元和纹理对象进行绑定
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
//    glActiveTexture(GL_TEXTURE1);
//    glBindTexture(GL_TEXTURE_2D, texture2);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
//    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    //    glDrawArrays(GL_TRIANGLES, 0, 3);


    //窗口显示，交换双缓冲区
    eglSwapBuffers(display, winSurface);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    //释放着色器程序对象
    shader.release();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_loadYuvWithFilterEffect(JNIEnv *env, jobject thiz,
                                                                   jobject surface,
                                                                   jobject asset_manager,
                                                                   jint filter_type,
                                                                   jboolean isNeedScaleAnim = false) {
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    const char *vertexShaderString;
    const char *fragShaderString;

    switch (filter_type) {
        case filter_type_gray:
            vertexShaderString = vertexShaderWithMatrix;
            fragShaderString = fragYUV420PGray;
            break;

        case filter_type_oppo:
            vertexShaderString = vertexShaderWithMatrix;
            fragShaderString = fragYUV420POppositeColor;
            break;
        case filter_type_oppo_gray:
            vertexShaderString = vertexShaderWithMatrix;
            fragShaderString = fragYUV420POppoColorAndGray;
            break;

        case filter_type_divide_2:
            vertexShaderString = vertexShaderWithMatrix;
            fragShaderString = fragYUV420PDivideTo2;
            break;
        case filter_type_divide_4:
            vertexShaderString = vertexShaderWithMatrix;
            fragShaderString = fragYUV420PDivideTo4;
            break;

        default:
            vertexShaderString = vertexShaderWithMatrix;
            fragShaderString = fragYUV420P;
            break;
    }

    Shader shader(vertexShaderString,fragShaderString);
    int program = shader.use();

    //加入三维顶点数据
    static float ver[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f
    };

    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));
    glEnableVertexAttribArray(apos);
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 0, ver);

    //加入纹理坐标数据
    static float fragment[] = {
            1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
    };
    GLuint aTex = static_cast<GLuint>(glGetAttribLocation(program, "aTextCoord"));
    glEnableVertexAttribArray(aTex);
    glVertexAttribPointer(aTex, 2, GL_FLOAT, GL_FALSE, 0, fragment);

    GLint uScaleMatrixLocation = glGetUniformLocation(program, "uMatrix");
    mat4 scaleMatrix = glm::mat4(1.0f);

    int width = 640;
    int height = 272;

    //纹理初始化
    //对sampler变量，使用函数glUniform1i和glUniform1iv进行设置
    glUniform1i(glGetUniformLocation(program, "yTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "uTexture"), 1);
    glUniform1i(glGetUniformLocation(program, "vTexture"), 2);
    //纹理ID
    GLuint textures[3] = {0};
    //创建若干个纹理对象，并且得到纹理ID
    glGenTextures(3, textures);

    //绑定纹理。后面的的设置和加载全部作用于当前绑定的纹理对象
    //GL_TEXTURE0、GL_TEXTURE1、GL_TEXTURE2 的就是纹理单元，GL_TEXTURE_1D、GL_TEXTURE_2D、CUBE_MAP为纹理目标
    //通过 glBindTexture 函数将纹理目标和纹理绑定后，对纹理目标所进行的操作都反映到对纹理上
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //放大的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    // 加载纹理到 OpenGL，读入 buffer 定义的位图数据，并把它复制到当前绑定的纹理对象
    // 当前绑定的纹理对象就会被附加上纹理图像。
    //width,height表示每几个像素公用一个yuv元素？比如width / 2表示横向每两个像素使用一个元素？
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本 默认0
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图（这里就是只取一个亮度的颜色通道的意思）
                 width,//加载的纹理宽度。最好为2的次幂(这里对y分量数据当做指定尺寸算，但显示尺寸会拉伸到全屏？)
                 height,//加载的纹理高度。最好为2的次幂
                 0,//纹理边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图
                 GL_UNSIGNED_BYTE,//像素点存储的数据类型
                 NULL //纹理的数据（先不传）
    );

    //绑定纹理
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本 默认0
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图（这里就是只取一个颜色通道的意思）
                 width / 2,//u数据数量为屏幕的4分之1
                 height / 2,
                 0,//边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图
                 GL_UNSIGNED_BYTE,//像素点存储的数据类型
                 NULL //纹理的数据（先不传）
    );

    //绑定纹理
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本 默认0
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图（这里就是只取一个颜色通道的意思）
                 width / 2,
                 height / 2,//v数据数量为屏幕的4分之1
                 0,//边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图
                 GL_UNSIGNED_BYTE,//像素点存储的数据类型
                 NULL //纹理的数据（先不传）
    );

    //创建3个buffer数组分别用于存放YUV三个分量
    unsigned char *buf[3] = {0};
    buf[0] = new unsigned char[width * height];//y
    buf[1] = new unsigned char[width * height / 4];//u
    buf[2] = new unsigned char[width * height / 4];//v

    //得到AAssetManager对象指针
    AAssetManager *mManeger = AAssetManager_fromJava(env, asset_manager);
    //得到AAsset对象
    AAsset *dataAsset = AAssetManager_open(mManeger, "video1_640_272.yuv",
                                           AASSET_MODE_STREAMING);//get file read AAsset
    //文件总长度
    off_t dataBufferSize = AAsset_getLength(dataAsset);
    //纵帧数
    long frameCount = dataBufferSize / (width * height * 3 / 2);

    BITMAP_INFO_LOGD("frameCount:%d", frameCount);

    int scaleDuration = frameCount / 10;

    for (int i = 0; i < frameCount; ++i) {
        //读取y分量
        int bufYRead = AAsset_read(dataAsset, buf[0],
                                   width * height);  //begin to read data once time
        //读取u分量
        int bufURead = AAsset_read(dataAsset, buf[1],
                                   width * height / 4);  //begin to read data once time
        //读取v分量
        int bufVRead = AAsset_read(dataAsset, buf[2],
                                   width * height / 4);  //begin to read data once time
        BITMAP_INFO_LOGD("bufYRead:%d,bufURead:%d,bufVRead:%d", bufYRead, bufURead, bufVRead);

        //读到文件末尾了
        if (bufYRead <= 0 || bufURead <= 0 || bufVRead <= 0) {
            AAsset_close(dataAsset);
            return;
        }


        if (isNeedScaleAnim) {
            float scale = getTransformScale(scaleDuration, i);

            //vec3(scale)的3个分量分别乘以scaleMatrix的前三行，第四行齐次坐标不变
            mat4 resultMatrix = glm::scale(scaleMatrix, vec3(scale));
            //最后一个参数是围绕哪个向量旋转
//        resultMatrix = glm::rotate(scaleMatrix, glm::radians(180.0f - scale * 180.0f),
//                                   glm::vec3(0.0f, 0.0f, 1.0f));
//        resultMatrix = glm::translate(glm::vec3(0.5f, 0.5f, 0.0f));
            glUniformMatrix4fv(uScaleMatrixLocation, 1, GL_FALSE, glm::value_ptr(resultMatrix));
        }


//        BITMAP_INFO_LOGD("resultMatrix:%d,bufURead:%d,bufVRead:%d", resultMatrix, , bufVRead);

        //  int c = dataRead(mManeger, "video1_640_272.yuv");

        //激活第一层纹理，绑定到创建的纹理
        //下面的width,height主要是显示尺寸？
        glActiveTexture(GL_TEXTURE0);
        //绑定y对应的纹理
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        //替换纹理，比重新使用glTexImage2D性能高多
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, 0,//相对原来的纹理的offset
                        width, height,//加载的纹理宽度、高度。最好为2的次幂
                        GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        buf[0]);

        //激活第二层纹理，绑定到创建的纹理
        glActiveTexture(GL_TEXTURE1);
        //绑定u对应的纹理
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        //替换纹理，比重新使用glTexImage2D性能高
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                        GL_UNSIGNED_BYTE,
                        buf[1]);

        //激活第三层纹理，绑定到创建的纹理
        glActiveTexture(GL_TEXTURE2);
        //绑定v对应的纹理
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        //替换纹理，比重新使用glTexImage2D性能高
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                        GL_UNSIGNED_BYTE,
                        buf[2]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //窗口显示，交换双缓冲区
        eglSwapBuffers(display, winSurface);

        //加一点延时效果避免帧率过快
        usleep(20000);
    }

    AAsset_close(dataAsset);

    shader.release();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_loadYuvWithSoulFled(JNIEnv *env, jobject thiz,
                                                               jobject surface,
                                                               jobject asset_manager) {
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    const char *vertexShaderString;
    const char *fragShaderString;


    vertexShaderString = vertexShaderWithMatrix;
    fragShaderString = fragSoulFled;

    Shader shader(vertexShaderString,fragShaderString);
    int program = shader.use();

    //加入三维顶点数据
    static float ver[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f
    };

    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));
    glEnableVertexAttribArray(apos);
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 0, ver);

    //加入纹理坐标数据
    static float fragment[] = {
            1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
    };
    GLuint aTex = static_cast<GLuint>(glGetAttribLocation(program, "aTextCoord"));
    glEnableVertexAttribArray(aTex);
    glVertexAttribPointer(aTex, 2, GL_FLOAT, GL_FALSE, 0, fragment);

    GLint uVetexMatrixLocation = glGetUniformLocation(program, "uMatrix");
    GLint uScaleLocation = glGetUniformLocation(program, "uScale");
    mat4 vetexMatrix = glm::mat4(1.0f);
    mat4 scaleMatrix = glm::mat4(1.0f);

    int width = 640;
    int height = 272;

    //纹理初始化
    //对sampler变量，使用函数glUniform1i和glUniform1iv进行设置
    glUniform1i(glGetUniformLocation(program, "yTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "uTexture"), 1);
    glUniform1i(glGetUniformLocation(program, "vTexture"), 2);
    //纹理ID
    GLuint textures[3] = {0};
    //创建若干个纹理对象，并且得到纹理ID
    glGenTextures(3, textures);

    //绑定纹理。后面的的设置和加载全部作用于当前绑定的纹理对象
    //GL_TEXTURE0、GL_TEXTURE1、GL_TEXTURE2 的就是纹理单元，GL_TEXTURE_1D、GL_TEXTURE_2D、CUBE_MAP为纹理目标
    //通过 glBindTexture 函数将纹理目标和纹理绑定后，对纹理目标所进行的操作都反映到对纹理上
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //放大的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    // 加载纹理到 OpenGL，读入 buffer 定义的位图数据，并把它复制到当前绑定的纹理对象
    // 当前绑定的纹理对象就会被附加上纹理图像。
    //width,height表示每几个像素公用一个yuv元素？比如width / 2表示横向每两个像素使用一个元素？
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本 默认0
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图（这里就是只取一个亮度的颜色通道的意思）
                 width,//加载的纹理宽度。最好为2的次幂(这里对y分量数据当做指定尺寸算，但显示尺寸会拉伸到全屏？)
                 height,//加载的纹理高度。最好为2的次幂
                 0,//纹理边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图
                 GL_UNSIGNED_BYTE,//像素点存储的数据类型
                 NULL //纹理的数据（先不传）
    );

    //绑定纹理
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本 默认0
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图（这里就是只取一个颜色通道的意思）
                 width / 2,//u数据数量为屏幕的4分之1
                 height / 2,
                 0,//边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图
                 GL_UNSIGNED_BYTE,//像素点存储的数据类型
                 NULL //纹理的数据（先不传）
    );

    //绑定纹理
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本 默认0
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图（这里就是只取一个颜色通道的意思）
                 width / 2,
                 height / 2,//v数据数量为屏幕的4分之1
                 0,//边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图
                 GL_UNSIGNED_BYTE,//像素点存储的数据类型
                 NULL //纹理的数据（先不传）
    );

    //创建3个buffer数组分别用于存放YUV三个分量
    unsigned char *buf[3] = {0};
    buf[0] = new unsigned char[width * height];//y
    buf[1] = new unsigned char[width * height / 4];//u
    buf[2] = new unsigned char[width * height / 4];//v

    //得到AAssetManager对象指针
    AAssetManager *mManeger = AAssetManager_fromJava(env, asset_manager);
    //得到AAsset对象
    AAsset *dataAsset = AAssetManager_open(mManeger, "video1_640_272.yuv",
                                           AASSET_MODE_STREAMING);//get file read AAsset
    //文件总长度
    off_t dataBufferSize = AAsset_getLength(dataAsset);
    //纵帧数
    long frameCount = dataBufferSize / (width * height * 3 / 2);

    BITMAP_INFO_LOGD("frameCount:%d", frameCount);

    int scaleDuration = frameCount / 10;

    for (int i = 0; i < frameCount; ++i) {
        //读取y分量
        int bufYRead = AAsset_read(dataAsset, buf[0],
                                   width * height);  //begin to read data once time
        //读取u分量
        int bufURead = AAsset_read(dataAsset, buf[1],
                                   width * height / 4);  //begin to read data once time
        //读取v分量
        int bufVRead = AAsset_read(dataAsset, buf[2],
                                   width * height / 4);  //begin to read data once time
        BITMAP_INFO_LOGD("bufYRead:%d,bufURead:%d,bufVRead:%d", bufYRead, bufURead, bufVRead);

        //读到文件末尾了
        if (bufYRead <= 0 || bufURead <= 0 || bufVRead <= 0) {
            AAsset_close(dataAsset);
            return;
        }


        float scale = getTransformRatio(scaleDuration, i);

        //vec3(scale)的3个分量分别乘以scaleMatrix的前三行，第四行齐次坐标不变
//        mat4 resultMatrix = glm::scale(scaleMatrix, vec3(scale));
        //最后一个参数是围绕哪个向量旋转
//        resultMatrix = glm::rotate(scaleMatrix, glm::radians(180.0f - scale * 180.0f),
//                                   glm::vec3(0.0f, 0.0f, 1.0f));
//        resultMatrix = glm::translate(glm::vec3(0.5f, 0.5f, 0.0f));
        glUniformMatrix4fv(uVetexMatrixLocation, 1, GL_FALSE, glm::value_ptr(vetexMatrix));
//        glUniformMatrix4fv(uScaleLocation, 1, GL_FALSE, glm::value_ptr(scaleMatrix));
        glUniform1f(uScaleLocation, scale);


//        BITMAP_INFO_LOGD("resultMatrix:%d,bufURead:%d,bufVRead:%d", resultMatrix, , bufVRead);

        //  int c = dataRead(mManeger, "video1_640_272.yuv");

        //激活第一层纹理，绑定到创建的纹理
        //下面的width,height主要是显示尺寸？
        glActiveTexture(GL_TEXTURE0);
        //绑定y对应的纹理
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        //替换纹理，比重新使用glTexImage2D性能高多
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, 0,//相对原来的纹理的offset
                        width, height,//加载的纹理宽度、高度。最好为2的次幂
                        GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        buf[0]);

        //激活第二层纹理，绑定到创建的纹理
        glActiveTexture(GL_TEXTURE1);
        //绑定u对应的纹理
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        //替换纹理，比重新使用glTexImage2D性能高
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                        GL_UNSIGNED_BYTE,
                        buf[1]);

        //激活第三层纹理，绑定到创建的纹理
        glActiveTexture(GL_TEXTURE2);
        //绑定v对应的纹理
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        //替换纹理，比重新使用glTexImage2D性能高
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                        GL_UNSIGNED_BYTE,
                        buf[2]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //窗口显示，交换双缓冲区
        eglSwapBuffers(display, winSurface);

        //加一点延时效果避免帧率过快
        usleep(20000);
    }

    AAsset_close(dataAsset);

    shader.release();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_loadYuvWithBlurEffect(JNIEnv *env, jobject thiz,
                                                                 jobject surface,
                                                                 jobject asset_manager,
                                                                 jint filter_type) {
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    const char *vertexShaderString;
    const char *fragShaderString;

//    switch (filter_type) {
//        case filter_type_gray:
//            vertexShaderString = vertexShaderWithMatrix;
//            fragShaderString = fragYUV420PGray;
//            break;
//
//        case filter_type_oppo:
//            vertexShaderString = vertexShaderWithMatrix;
//            fragShaderString = fragYUV420POppositeColor;
//            break;
//        case filter_type_oppo_gray:
//            vertexShaderString = vertexShaderWithMatrix;
//            fragShaderString = fragYUV420POppoColorAndGray;
//            break;
//
//        case filter_type_divide_2:
//            vertexShaderString = vertexShaderWithMatrix;
//            fragShaderString = fragYUV420PDivideTo2;
//            break;
//        case filter_type_divide_4:
//            vertexShaderString = vertexShaderWithMatrix;
//            fragShaderString = fragYUV420PDivideTo4;
//            break;
//
//        default:
//            vertexShaderString = vertexShaderWithMatrix;
//            fragShaderString = fragYUV420P;
//            break;
//    }

    Shader shader(GAUSSIAN_BLUR_VERTEX_SHADER,GAUSSIAN_BLUR_FRAGMENT_SHADER);
    int program = shader.use();

    //加入三维顶点数据
    static float ver[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f
    };

    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));
    glEnableVertexAttribArray(apos);
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 0, ver);

    //加入纹理坐标数据
    static float fragment[] = {
            1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
    };
    GLuint aTex = static_cast<GLuint>(glGetAttribLocation(program, "aTextCoord"));
    glEnableVertexAttribArray(aTex);
    glVertexAttribPointer(aTex, 2, GL_FLOAT, GL_FALSE, 0, fragment);

    GLint uScaleMatrixLocation = glGetUniformLocation(program, "uMatrix");
    mat4 scaleMatrix = glm::mat4(1.0f);

    int width = 640;
    int height = 272;

    //纹理初始化
    //对sampler变量，使用函数glUniform1i和glUniform1iv进行设置
    glUniform1i(glGetUniformLocation(program, "yTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "uTexture"), 1);
    glUniform1i(glGetUniformLocation(program, "vTexture"), 2);
    //纹理ID
    GLuint textures[3] = {0};
    //创建若干个纹理对象，并且得到纹理ID
    glGenTextures(3, textures);

    //绑定纹理。后面的的设置和加载全部作用于当前绑定的纹理对象
    //GL_TEXTURE0、GL_TEXTURE1、GL_TEXTURE2 的就是纹理单元，GL_TEXTURE_1D、GL_TEXTURE_2D、CUBE_MAP为纹理目标
    //通过 glBindTexture 函数将纹理目标和纹理绑定后，对纹理目标所进行的操作都反映到对纹理上
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //放大的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    // 加载纹理到 OpenGL，读入 buffer 定义的位图数据，并把它复制到当前绑定的纹理对象
    // 当前绑定的纹理对象就会被附加上纹理图像。
    //width,height表示每几个像素公用一个yuv元素？比如width / 2表示横向每两个像素使用一个元素？
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本 默认0
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图（这里就是只取一个亮度的颜色通道的意思）
                 width,//加载的纹理宽度。最好为2的次幂(这里对y分量数据当做指定尺寸算，但显示尺寸会拉伸到全屏？)
                 height,//加载的纹理高度。最好为2的次幂
                 0,//纹理边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图
                 GL_UNSIGNED_BYTE,//像素点存储的数据类型
                 NULL //纹理的数据（先不传）
    );

    //绑定纹理
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本 默认0
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图（这里就是只取一个颜色通道的意思）
                 width / 2,//u数据数量为屏幕的4分之1
                 height / 2,
                 0,//边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图
                 GL_UNSIGNED_BYTE,//像素点存储的数据类型
                 NULL //纹理的数据（先不传）
    );

    //绑定纹理
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本 默认0
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图（这里就是只取一个颜色通道的意思）
                 width / 2,
                 height / 2,//v数据数量为屏幕的4分之1
                 0,//边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图
                 GL_UNSIGNED_BYTE,//像素点存储的数据类型
                 NULL //纹理的数据（先不传）
    );

    //创建3个buffer数组分别用于存放YUV三个分量
    unsigned char *buf[3] = {0};
    buf[0] = new unsigned char[width * height];//y
    buf[1] = new unsigned char[width * height / 4];//u
    buf[2] = new unsigned char[width * height / 4];//v

    //得到AAssetManager对象指针
    AAssetManager *mManeger = AAssetManager_fromJava(env, asset_manager);
    //得到AAsset对象
    AAsset *dataAsset = AAssetManager_open(mManeger, "video1_640_272.yuv",
                                           AASSET_MODE_STREAMING);//get file read AAsset
    //文件总长度
    off_t dataBufferSize = AAsset_getLength(dataAsset);
    //纵帧数
    long frameCount = dataBufferSize / (width * height * 3 / 2);

    BITMAP_INFO_LOGD("frameCount:%d", frameCount);

    int scaleDuration = frameCount / 10;

    for (int i = 0; i < frameCount; ++i) {
        //读取y分量
        int bufYRead = AAsset_read(dataAsset, buf[0],
                                   width * height);  //begin to read data once time
        //读取u分量
        int bufURead = AAsset_read(dataAsset, buf[1],
                                   width * height / 4);  //begin to read data once time
        //读取v分量
        int bufVRead = AAsset_read(dataAsset, buf[2],
                                   width * height / 4);  //begin to read data once time
        BITMAP_INFO_LOGD("bufYRead:%d,bufURead:%d,bufVRead:%d", bufYRead, bufURead, bufVRead);

        //读到文件末尾了
        if (bufYRead <= 0 || bufURead <= 0 || bufVRead <= 0) {
            AAsset_close(dataAsset);
            return;
        }

        float scale = getTransformScale(scaleDuration, i);

        //vec3(scale)的3个分量分别乘以scaleMatrix的前三行，第四行齐次坐标不变
//        mat4 resultMatrix = glm::scale(scaleMatrix, vec3(scale));
        //最后一个参数是围绕哪个向量旋转
//        resultMatrix = glm::rotate(scaleMatrix, glm::radians(180.0f - scale * 180.0f),
//                                   glm::vec3(0.0f, 0.0f, 1.0f));
//        resultMatrix = glm::translate(glm::vec3(0.5f, 0.5f, 0.0f));
//        glUniformMatrix4fv(uScaleMatrixLocation, 1, GL_FALSE, glm::value_ptr(resultMatrix));
        glUniformMatrix4fv(uScaleMatrixLocation, 1, GL_FALSE, glm::value_ptr(scaleMatrix));

//        BITMAP_INFO_LOGD("resultMatrix:%d,bufURead:%d,bufVRead:%d", resultMatrix, , bufVRead);

        //  int c = dataRead(mManeger, "video1_640_272.yuv");

        //激活第一层纹理，绑定到创建的纹理
        //下面的width,height主要是显示尺寸？
        glActiveTexture(GL_TEXTURE0);
        //绑定y对应的纹理
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        //替换纹理，比重新使用glTexImage2D性能高多
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, 0,//相对原来的纹理的offset
                        width, height,//加载的纹理宽度、高度。最好为2的次幂
                        GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        buf[0]);

        //激活第二层纹理，绑定到创建的纹理
        glActiveTexture(GL_TEXTURE1);
        //绑定u对应的纹理
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        //替换纹理，比重新使用glTexImage2D性能高
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                        GL_UNSIGNED_BYTE,
                        buf[1]);

        //激活第三层纹理，绑定到创建的纹理
        glActiveTexture(GL_TEXTURE2);
        //绑定v对应的纹理
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        //替换纹理，比重新使用glTexImage2D性能高
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                        GL_UNSIGNED_BYTE,
                        buf[2]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //窗口显示，交换双缓冲区
        eglSwapBuffers(display, winSurface);

        //加一点延时效果避免帧率过快
        usleep(20000);
    }

    AAsset_close(dataAsset);

    shader.release();
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_draw3DCubeTexture(JNIEnv *env, jobject thiz,
                                                             jobjectArray bitmaps,
                                                             jobject surface,
                                                             jint screen_width,
                                                             jint screen_height) {
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES3_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Shader shader(vertexShader3D,frag3DTexture);
    int program = shader.use();

    float vertices[] = {
            // 顶点坐标           纹理坐标
            //背面
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  //2
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  //1
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, //0
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  //0
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  //3
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  //2

            //前面
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,//4
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,//5
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,//6
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,//6
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,//7
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,//4

            //左面
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,//7
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,//3
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,//0
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,//0
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,//4
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,//7

            //右面
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,//1
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,//2
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,//6
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,//6
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,//5
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,//1

            //底面
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,//0
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,//1
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,//5
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,//5
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,//4
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,//0

            //上面
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,//6
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,//2
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,//3
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,//3
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,//7
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,//6

    };
//    unsigned int indices[] = {
//            //背面
//            0, 1, 3, // first triangle
//            1, 2, 3, // second triangle
//            //上面
//            2, 3, 7, // first triangle
//            7, 6, 2,  // second triangle
//            //左面
//            3, 0, 4, // first triangle
//            4, 7, 3, // second triangle
//            //右面
//            5, 1, 2, // first triangle
//            2, 6, 5, // second triangle
//            //下面
//            1, 0, 4, // first triangle
//            4, 5, 1, // second triangle
//            //前面
//            4, 5, 6, // first triangle
//            6, 7, 4, // second triangle
//    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
//    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    BITMAP_INFO_LOGD("glBufferData GL_ELEMENT_ARRAY_BUFFER");

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    BITMAP_INFO_LOGD("glEnableVertexAttribArray(1)");

    std::vector<BitmapInfo> bitmapVector;
    jsize ref_size = env->GetArrayLength(bitmaps);
    for (int i = 0; i < ref_size; ++i) {
        jobject bitmap = env->GetObjectArrayElement(bitmaps, i);
        BITMAP_INFO_LOGD("bitmapVector.push_back(bitmapInfo) &bitmap:%p ,i：%d:" , &bitmap, i);
        AndroidBitmapInfo bmpInfo;
        BitmapInfo bitmapInfo(env,bitmap,bmpInfo);
        bitmapVector.emplace_back(bitmapInfo);

    }


    // load and create a texture
    // -------------------------
//    unsigned int texture1, texture2;
    unsigned int texture1;
    //-------------------- texture1的配置start ------------------------------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters（配置纹理环绕）
    //横坐标环绕配置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_REPEAT);    // set texture wrapping to GL_REPEAT (default wrapping method)
    //纵坐标环绕配置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters（配置纹理过滤）
    //纹理分辨率大于图元分辨率，即纹理需要被缩小的过滤配置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //纹理分辨率小于图元分辨率，即纹理需要被放大的过滤配置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    //-------------------- texture1的配置end ------------------------------

    //对着色器中的纹理单元变量进行赋值
    glUniform1i(glGetUniformLocation(program, "ourTexture"), 0);
//    glUniform1i(glGetUniformLocation(program, "ourTexture1"), 1);

    //将纹理单元和纹理对象进行绑定
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, texture1);

    float f = 0.0f;
    while (f >= 0) {
        //模型矩阵，将局部坐标转换为世界坐标
        glm::mat4 model = glm::mat4(1.0f);
        //视图矩阵，确定物体和摄像机的相对位置
        glm::mat4 view = glm::mat4(1.0f);
        //透视投影矩阵，实现近大远小的效果
        glm::mat4 projection = glm::mat4(1.0f);
        //沿着向量(0.5f, 1.0f, 0.0f)旋转
        model = glm::rotate(model, glm::radians(f), glm::vec3(0.5f, 1.0f, 0.0f));
        // 注意，我们将矩阵向我们要进行移动场景的反方向移动。（右手坐标系，所以z正方形从屏幕指向外部）
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
        BITMAP_INFO_LOGD("glm::perspective:%d,height:%d", screen_width, screen_height);
        projection = glm::perspective(glm::radians(45.0f),
                                      (float) screen_width / (float) screen_height, 0.1f,
                                      100.0f);
        BITMAP_INFO_LOGD("mat4 init");
        GLint modelLoc = glGetUniformLocation(program, "model");
        GLint viewLoc = glGetUniformLocation(program, "view");
        GLint projectionLoc = glGetUniformLocation(program, "projection");
        BITMAP_INFO_LOGD("glGetUniformLocation");
        // pass them to the shaders (3 different ways)
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        BITMAP_INFO_LOGD("glUniformMatrix4fv");

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    glBindVertexArray(VAO);
//    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

//    glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindVertexArray(VAO);
        for (int i = 0; i < 36; i = i + 6) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
//            glTexSubImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmpInfo.width, bmpInfo.height, 0, GL_RGBA,
//                         GL_UNSIGNED_BYTE, bmpPixels);
            int index = i/6;
            BITMAP_INFO_LOGD("index:%d", index);
            BitmapInfo bmpInfo = bitmapVector[index];
            void *bmpPixels;
            AndroidBitmap_lockPixels(env, bmpInfo.bitmap, &bmpPixels);
            BITMAP_INFO_LOGD("bmpInfo.bitmap：%p，bmpInfo.bmpInfo.width:%d,bmpInfo.bmpInfo.height:%d", bmpInfo.bitmap
                 , bmpInfo.bmpInfo.width, bmpInfo.bmpInfo.height);
            //替换纹理，比重新使用glTexImage2D性能高
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bmpInfo.bmpInfo.width, bmpInfo.bmpInfo.height, GL_RGBA,
                            GL_UNSIGNED_BYTE,
                            bmpPixels);
            AndroidBitmap_unlockPixels(env, bmpInfo.bitmap);
            glDrawArrays(GL_TRIANGLES, i, 6);
        }

        //窗口显示，交换双缓冲区
        eglSwapBuffers(display, winSurface);

        f++;
        sleep(static_cast<unsigned int>(0.4f));
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
//    glDeleteBuffers(1, &EBO);
    //释放着色器程序对象
    shader.release();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_draw3DCubeWithColor(JNIEnv *env, jobject thiz,
                                                               jobject surface, jint screen_width,
                                                               jint screen_height) {
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES3_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Shader shader(vertexShader3DGradientColor,frag3DGradientColor);
    int program = shader.use();

    float vertices[] = {
            // 顶点坐标                 颜色
            -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,//背后左下角点 0
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,//背后右下角点 1

            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f,//背后右上角点 2
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f,//背后左上角点 3

            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,//前面左下角点 4
            0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 1.0f,//前面右下角点 5

            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,//前面右上角点 6
            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,//前面左上角点 7

    };
    unsigned int indices[] = {
            //背面
            0, 3, 1, // first triangle
            3, 2, 1, // second triangle
            //上面
            2, 3, 7, // first triangle
            7, 6, 2,  // second triangle
            //左面
            3, 0, 4, // first triangle
            4, 7, 3, // second triangle
            //右面
            5, 1, 2, // first triangle
            2, 6, 5, // second triangle
            //下面
            4, 0, 1, // first triangle
            1, 5, 4,// second triangle
            //前面
            4, 5, 6, // first triangle
            6, 7, 4, // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    BITMAP_INFO_LOGD("glBufferData GL_ELEMENT_ARRAY_BUFFER");

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float f = 0.0f;
    while (f >= 0) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //模型矩阵，将局部坐标转换为世界坐标
        glm::mat4 model = glm::mat4(1.0f);
        //视图矩阵，确定物体和摄像机的相对位置
        glm::mat4 view = glm::mat4(1.0f);
        //透视投影矩阵，实现近大远小的效果
        glm::mat4 projection = glm::mat4(1.0f);
        //沿着向量(0.5f, 1.0f, 0.0f)旋转
        BITMAP_INFO_LOGD("f:%f", f);
        model = glm::rotate(model, glm::radians(f), glm::vec3(0.5f, 1.0f, 0.0f));
        // 注意，我们将矩阵向我们要进行移动场景的反方向移动。（右手坐标系，所以z正方形从屏幕指向外部）
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));
        BITMAP_INFO_LOGD("glm::perspective:%d,height:%d", screen_width, screen_height);
        projection = glm::perspective(glm::radians(45.0f),
                                      (float) screen_width / (float) screen_height, 0.1f,
                                      100.0f);

        GLint modelLoc = glGetUniformLocation(program, "model");
        GLint viewLoc = glGetUniformLocation(program, "view");
        GLint projectionLoc = glGetUniformLocation(program, "projection");
        BITMAP_INFO_LOGD("glGetUniformLocation");
        // pass them to the shaders (3 different ways)
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        BITMAP_INFO_LOGD("glUniformMatrix4fv");

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //窗口显示，交换双缓冲区
        eglSwapBuffers(display, winSurface);

        f++;
        sleep(static_cast<unsigned int>(0.4));
    }


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    //释放着色器程序对象
    shader.release();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_draw3DColorCubeCamera(JNIEnv *env, jobject thiz,
                                                                 jobject surface, jint screen_width,
                                                                 jint screen_height) {
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        BITMAP_INFO_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        BITMAP_INFO_LOGD("eglInitialize failed");
        return;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES3_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        BITMAP_INFO_LOGD("eglChooseConfig failed");
        return;
    }

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        BITMAP_INFO_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        BITMAP_INFO_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        BITMAP_INFO_LOGD("eglMakeCurrent failed");
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(true);

    Shader shader(vertexShader3DGradientColor,frag3DGradientColor);
    int program = shader.use();

    // world space positions of our cubes
    glm::vec3 cubePositions[] = {
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3 (2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3( 1.3f, -2.0f, -2.5f),
            glm::vec3( 1.5f,  2.0f, -2.5f),
            glm::vec3( 1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f),
            glm::vec3( 0.0f,  0.0f,  0.0f),

    };

    float vertices[] = {
            // 顶点坐标                 颜色
            -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,//背后左下角点 0
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,//背后右下角点 1

            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f,//背后右上角点 2
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f,//背后左上角点 3

            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,//前面左下角点 4
            0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 1.0f,//前面右下角点 5

            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,//前面右上角点 6
            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,//前面左上角点 7

    };
    unsigned int indices[] = {
            //背面
            0, 3, 1, // first triangle
            3, 2, 1, // second triangle
            //上面
            2, 3, 7, // first triangle
            7, 6, 2,  // second triangle
            //左面
            3, 0, 4, // first triangle
            4, 7, 3, // second triangle
            //右面
            5, 1, 2, // first triangle
            2, 6, 5, // second triangle
            //下面
            4, 0, 1, // first triangle
            1, 5, 4,// second triangle
            //前面
            4, 5, 6, // first triangle
            6, 7, 4, // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    BITMAP_INFO_LOGD("glBufferData GL_ELEMENT_ARRAY_BUFFER");

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float f = 0.0f;
    while (f >= 0) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //模型矩阵，将局部坐标转换为世界坐标
//        glm::mat4 model = glm::mat4(1.0f);
        //视图矩阵，确定物体和摄像机的相对位置
        glm::mat4 view = glm::mat4(1.0f);
        //透视投影矩阵，实现近大远小的效果
        glm::mat4 projection = glm::mat4(1.0f);
        //沿着向量(0.5f, 1.0f, 0.0f)旋转
        BITMAP_INFO_LOGD("f:%f", f);
//        model = glm::rotate(model, glm::radians(f), glm::vec3(0.5f, 1.0f, 0.0f));
        // glm::LookAt函数需要一个位置、目标和上向量，创造一个观察矩阵
//        view = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f),//位置
//                           glm::vec3(0.0f, 0.0f, 0.0f),//目标
//                           glm::vec3(0.0f, 1.0f, 0.0f));//上向量


        float radius = 10.0f;
        float camX = sin(f/100.0f) * radius;
        float camZ = cos(f/100.0f) * radius;
        view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

        BITMAP_INFO_LOGD("glm::perspective:%d,height:%d", screen_width, screen_height);
        projection = glm::perspective(glm::radians(45.0f),
                                      (float) screen_width / (float) screen_height, 0.1f,
                                      100.0f);

        GLint modelLoc = glGetUniformLocation(program, "model");
        GLint viewLoc = glGetUniformLocation(program, "view");
        GLint projectionLoc = glGetUniformLocation(program, "projection");
        BITMAP_INFO_LOGD("glGetUniformLocation");
        // pass them to the shaders (3 different ways)
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        BITMAP_INFO_LOGD("glUniformMatrix4fv");

        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
//            float angle = 20.0f * i;
            switch (i % 3) {
                case 0:
                    model = glm::rotate(model, glm::radians(f*2), glm::vec3(1.0f, 0.3f, 0.5f));
                    break;
                case 1:
                    model = glm::rotate(model, glm::radians(f), glm::vec3(1.0f, 1.0f, 0.5f));
                    break;
                case 2:
                    model = glm::rotate(model, glm::radians(f*1.5f), glm::vec3(0.5f, 0.0f, 0.5f));
                    break;
            }
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        //窗口显示，交换双缓冲区
        eglSwapBuffers(display, winSurface);

        f++;
        sleep(static_cast<unsigned int>(0.4));
    }


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    //释放着色器程序对象
    shader.release();
}



