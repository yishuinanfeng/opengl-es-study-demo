//
// Created by 易水南风 on 2023/5/22.
//

#ifndef OPENGLSTUDYDEMO_TOUCHCTLCAMERA_H
#define OPENGLSTUDYDEMO_TOUCHCTLCAMERA_H

#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include "FragmentShader.h"
#include "glm/glm/gtc/matrix_transform.hpp"
#include "glm/glm/ext.hpp"
#include "glm/glm/detail/_noise.hpp"

#define TouchCtlCamera_LOGD(...) __android_log_print(ANDROID_LOG_WARN,"TouchCtlCamera",__VA_ARGS__)


class TouchCtlCamera {
public:

    float yaw = -90.0f;    // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
    float pitch = 0.0f;
    float lastX = 0.0f;
    float lastY = 0.0f;
    float fov = 45.0f;

// camera
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -10.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);


    void draw3DCubesCameraTouchCtl(JNIEnv *env, jobject thiz,
                                   jobject surface,
                                   jint screen_width,
                                   jint screen_height);

};

enum TouchActionMode {
    ACTION_DOWN = 0,
    ACTION_UP = 1,
    ACTION_MOVE = 2,
    ACTION_CANCEL = 3
};


#endif //OPENGLSTUDYDEMO_TOUCHCTLCAMERA_H
