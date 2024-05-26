package com.example.openglstudydemo

import android.util.Log

/**
 * 全局配置类
 */
class Config {
    companion object {
        private const val TAG = "Config"
        val VideoTypeKey = "VideoType"

        val videoTypeList = mutableListOf<VideoTypeBean>().apply {
            Log.d("VideoTypeAdapter", "init videoTypeList")
            add(VideoTypeBean(VideoTypeEnum.SIMPLE_YUV, "基础YUV视频"))
            add(VideoTypeBean(VideoTypeEnum.NO_FILTER_YUV, "无滤镜YUV视频"))
            add(VideoTypeBean(VideoTypeEnum.GRAY_FILTER_YUV, "灰度滤镜视频"))
            add(VideoTypeBean(VideoTypeEnum.OPPO_FILTER_YUV, "反色滤镜视频"))
            add(VideoTypeBean(VideoTypeEnum.DIVIDE_TO_2_YUV, "二分屏滤镜视频"))
            add(VideoTypeBean(VideoTypeEnum.DIVIDE_TO_4_YUV, "四分屏滤镜视频"))
            add(VideoTypeBean(VideoTypeEnum.SOUL_FLY_OFF_YUV, "灵魂出窍滤镜视频"))
            add(VideoTypeBean(VideoTypeEnum.GAUSS_BLUR_YUV, "高斯模糊滤镜视频"))
            add(VideoTypeBean(VideoTypeEnum.DRAW_TRIANGLE, "三角形"))
            add(VideoTypeBean(VideoTypeEnum.DRAW_TWO_TRIANGLE, "两个三角形"))
            add(VideoTypeBean(VideoTypeEnum.DRAW_LINE, "绘制线段"))
            add(VideoTypeBean(VideoTypeEnum.DRAW_TRIANGLE_UNIFORM, "通过uniform绘制三角形"))
            add(VideoTypeBean(VideoTypeEnum.DRAW_TRIANGLE_VBO, "使用vbo绘制三角形"))
            add(VideoTypeBean(VideoTypeEnum.DRAW_TRIANGLE_EBO, "使用ebo绘制三角形"))
            add(VideoTypeBean(VideoTypeEnum.DRAW_TEXTURE_MAX, "纹理"))
            add(VideoTypeBean(VideoTypeEnum.DRAW_3D_TEXTURE, "3D纹理"))
            add(VideoTypeBean(VideoTypeEnum.DRAW_3D_CUBE_TEXTURE, "带纹理方体"))
            add(VideoTypeBean(VideoTypeEnum.DRAW_GRADIENT_COLOR_CUBE_TEXTURE, "渐变色立方体"))
            add(VideoTypeBean(VideoTypeEnum.DRAW_CUBES_MOVE_CAMERA, "移动摄像机拍摄立方体"))

        }
    }


}