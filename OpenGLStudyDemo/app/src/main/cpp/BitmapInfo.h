//
// Created by 易水南风 on 2023/5/20.
//

#ifndef OPENGLSTUDYDEMO_BITMAPINFO_H
#define OPENGLSTUDYDEMO_BITMAPINFO_H

#define BITMAP_INFO_LOGD(...) __android_log_print(ANDROID_LOG_WARN,"BitmapInfo",__VA_ARGS__)


#include <android/log.h>

class BitmapInfo {
public:
    AndroidBitmapInfo bmpInfo;
    jobject bitmap;

public:
    BitmapInfo(JNIEnv *env,jobject bitmap,AndroidBitmapInfo bmpInfo);

};


#endif //OPENGLSTUDYDEMO_BITMAPINFO_H
