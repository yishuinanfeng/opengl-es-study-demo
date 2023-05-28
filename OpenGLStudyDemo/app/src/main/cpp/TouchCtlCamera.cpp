//
// Created by 易水南风 on 2023/5/22.
//

#include "Shader.h"
#include "TouchCtlCamera.h"
#include <jni.h>
#include <unistd.h>

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_handleTouchEvent(JNIEnv *env, jobject thiz, jint action,
                                                            jfloat xpos, jfloat ypos) {

    TouchCtlCamera_LOGD("handleTouchEvent action：%d,xpos：%f,ypos:%f:",action, xpos, ypos);

    switch (action) {
        case TouchActionMode::ACTION_DOWN:
            lastX = xpos;
            lastY = ypos;
            break;
        case TouchActionMode::ACTION_MOVE:
            float xoffset = xpos - lastX;
            float yoffset = lastY - ypos;
            lastX = xpos;
            lastY = ypos;

            float sensitivity = 0.02;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            //根据触摸的偏移量计算出角度的变化
            yaw += xoffset;
            pitch += yoffset;

            if (pitch > 89.0f) {
                pitch = 89.0f;
            }

            if (pitch < -89.0f) {
                pitch = -89.0f;
            }

            //计算出角度变化导致的方向向量的变化
            glm::vec3 front;
            front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            front.y = sin(glm::radians(pitch));
            front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            cameraFront = glm::normalize(front);
            break;
//        case TouchActionMode::ACTION_UP:
//            lastX = 0.0f;
//            lastY = 0.0f;
//            break;
//        case TouchActionMode::ACTION_CANCEL:
//            lastX = 0.0f;
//            lastY = 0.0f;
//            break;
    }

//    if (firstTouch) {
//        lastX = xpos;
//        lastY = ypos;
//        firstTouch = false;
//    }


}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_openglstudydemo_YuvPlayer_draw3DCubesCameraTouchCtl(JNIEnv *env, jobject thiz,
                                                                     jobject surface,
                                                                     jint screen_width,
                                                                     jint screen_height) {
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        TouchCtlCamera_LOGD("egl display failed");
        return;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        TouchCtlCamera_LOGD("eglInitialize failed");
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
        TouchCtlCamera_LOGD("eglChooseConfig failed");
        return;
    }

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        TouchCtlCamera_LOGD("eglCreateWindowSurface failed");
        return;
    }

    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    //EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        TouchCtlCamera_LOGD("eglCreateContext failed");
        return;
    }
    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        TouchCtlCamera_LOGD("eglMakeCurrent failed");
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Shader shader(vertexShader3DGradientColor, frag3DGradientColor);
    int program = shader.use();

    // world space positions of our cubes
    glm::vec3 cubePositions[] = {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(2.0f, 5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3(2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f, 3.0f, -7.5f),
            glm::vec3(1.3f, -2.0f, -2.5f),
            glm::vec3(1.5f, 2.0f, -2.5f),
            glm::vec3(1.5f, 0.2f, -1.5f),
            glm::vec3(-1.3f, 1.0f, -1.5f)
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

    TouchCtlCamera_LOGD("glBufferData GL_ELEMENT_ARRAY_BUFFER");

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
        TouchCtlCamera_LOGD("f:%f", f);
//        model = glm::rotate(model, glm::radians(f), glm::vec3(0.5f, 1.0f, 0.0f));
        // glm::LookAt函数需要一个位置、目标和上向量，创造一个观察矩阵
//        view = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f),//位置
//                           glm::vec3(0.0f, 0.0f, 0.0f),//目标
//                           glm::vec3(0.0f, 1.0f, 0.0f));//上向量

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        TouchCtlCamera_LOGD("glm::perspective:%d,height:%d", screen_width, screen_height);
        projection = glm::perspective(glm::radians(45.0f),
                                      (float) screen_width / (float) screen_height, 0.1f,
                                      100.0f);

        GLint modelLoc = glGetUniformLocation(program, "model");
        GLint viewLoc = glGetUniformLocation(program, "view");
        GLint projectionLoc = glGetUniformLocation(program, "projection");
        TouchCtlCamera_LOGD("glGetUniformLocation");
        // pass them to the shaders (3 different ways)
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        TouchCtlCamera_LOGD("glUniformMatrix4fv");


        for (unsigned int i = 0; i < 10; i++) {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
//            float angle = 20.0f * i;
            switch (i % 3) {
                case 0:
                    model = glm::rotate(model, glm::radians(f * 2), glm::vec3(1.0f, 0.3f, 0.5f));
                    break;
                case 1:
                    model = glm::rotate(model, glm::radians(f), glm::vec3(1.0f, 1.0f, 0.5f));
                    break;
                case 2:
                    model = glm::rotate(model, glm::radians(f * 1.5f), glm::vec3(0.5f, 0.0f, 0.5f));
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
