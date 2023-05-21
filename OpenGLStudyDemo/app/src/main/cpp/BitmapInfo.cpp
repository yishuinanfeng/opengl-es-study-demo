//
// Created by 易水南风 on 2023/5/20.
//

#include <android/bitmap.h>
#include "BitmapInfo.h"



BitmapInfo::BitmapInfo(JNIEnv *env,jobject bitmap,AndroidBitmapInfo bmpInfo){
    this->bitmap = bitmap;
    this->bmpInfo = bmpInfo;
    int get_info = AndroidBitmap_getInfo(env, bitmap, &this->bmpInfo);
    LOGD("BitmapInfo AndroidBitmap_getInfo %d", get_info);
    LOGD("BitmapInfo bitmap width:%d,height:%d", this->bmpInfo.width, this->bmpInfo.height);
}


