//
// Created by 易水南风 on 2023/5/27.
//

#ifndef OPENGLSTUDYDEMO_SHADER_H
#define OPENGLSTUDYDEMO_SHADER_H


#include <GLES3/gl3.h>
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_WARN,"Shader",__VA_ARGS__)

class Shader {

private:
    GLint program;
    const char* vertexShader;
    const char* fragmentShader;

    GLint initShader(const char *source, int type);

public:
    Shader(const char* vertexShader,const char* fragmentShader);

    int use();

    void release();

};


#endif //OPENGLSTUDYDEMO_SHADER_H
