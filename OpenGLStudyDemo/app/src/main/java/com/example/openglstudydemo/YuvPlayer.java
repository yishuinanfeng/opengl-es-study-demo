package com.example.openglstudydemo;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class YuvPlayer extends GLSurfaceView implements Runnable, SurfaceHolder.Callback, GLSurfaceView.Renderer {

    // private final static String PATH = "/sdcard/sintel_640_360.yuv";
    private final static String PATH = "/sdcard/video1_640_272.yuv";

    public YuvPlayer(Context context, AttributeSet attrs) {
        super(context, attrs);
        setRenderer(this);

        Log.d("YuvPlayer", "YuvPlayer");
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.d("YuvPlayer", "surfaceCreated");
        new Thread(this).start();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        surfaceWidth = w;
        surfaceHeight = h;
    }

    @Override
    public void run() {
        Log.d("YuvPlayer", "run");

        AssetManager assetManager = getContext().getAssets();
//        loadYuv(getHolder().getSurface(),assetManager);
//      loadYuvWithFilterEffect(getHolder().getSurface(),assetManager,FilterType.NONE);
 //      loadYuvWithFilterEffect(getHolder().getSurface(),assetManager,FilterType.GRAY);
//       loadYuvWithFilterEffect(getHolder().getSurface(),assetManager,FilterType.OPPO_GRAY);
//        loadYuvWithFilterEffect(getHolder().getSurface(),assetManager,FilterType.DIVIDE_TO_2);
//        loadYuvWithFilterEffect(getHolder().getSurface(),assetManager,FilterType.DIVIDE_TO_4,true);
//        loadYuvWithSoulFled(getHolder().getSurface(),assetManager);

//        loadYuvWithBlurEffect(getHolder().getSurface(),assetManager,0);


//     drawTwoTriangle(getHolder().getSurface());
//        drawTriangle(getHolder().getSurface());
//        drawLineWithColor(getHolder().getSurface());
//        drawTriangleUniform(getHolder().getSurface());
//        drawTriangleWithEBO(getHolder().getSurface());
//        drawTriangleWithBufferObj(getHolder().getSurface());

//        drawableTexture();

        drawaTexture();

        Log.d("YuvPlayer", "loadYuv");
    }

    private void drawaTexture() {
        Bitmap bitmap = ((BitmapDrawable) getResources().getDrawable(R.drawable.liyingai)).getBitmap();
        Bitmap bitmap1 = ((BitmapDrawable) getResources().getDrawable(R.drawable.shiyuanmeili2)).getBitmap();


//        BitmapFactory.Options options1 = new BitmapFactory.Options();
//        options1.inPreferredConfig = Bitmap.Config.RGB_565;
//        Bitmap bitmap = BitmapFactory.decodeResource(getResources(),R.drawable.animal,options1);


        int w = bitmap.getWidth(), h = bitmap.getHeight();
        int[] pix = new int[w * h];
        bitmap.getPixels(pix, 0, w, 0, 0, w, h);

        Log.d("YuvPlayer", "drawableTexture:" + "w:" + w+ ",h:" + h + "，pix:" + Arrays.toString(pix));


//        drawTexture(bitmap, w, h, getHolder().getSurface());
//        drawTexture(bitmap,bitmap1,getHolder().getSurface());
//        draw3DTexture(bitmap,bitmap1,getHolder().getSurface(),surfaceWidth,surfaceHeight);




        Bitmap labixiaoxin = ((BitmapDrawable) getResources().getDrawable(R.drawable.labixiaoxin1)).getBitmap();
        Bitmap gangtieshenbing = ((BitmapDrawable) getResources().getDrawable(R.drawable.gangtieshenbing)).getBitmap();
        Bitmap maohelaoshu = ((BitmapDrawable) getResources().getDrawable(R.drawable.maohelaoshu)).getBitmap();
        Bitmap kenan = ((BitmapDrawable) getResources().getDrawable(R.drawable.kenan)).getBitmap();
        Bitmap zuqiuxiaojiang = ((BitmapDrawable) getResources().getDrawable(R.drawable.zuqiuxiaojiang)).getBitmap();
        Bitmap qilongzhu = ((BitmapDrawable) getResources().getDrawable(R.drawable.qilongzhu)).getBitmap();


        Bitmap[] bitmaps = new Bitmap[6];
        bitmaps[0] = labixiaoxin;
        bitmaps[1] = gangtieshenbing;
        bitmaps[2] = maohelaoshu;
        bitmaps[3] = qilongzhu;
        bitmaps[4] = zuqiuxiaojiang;
        bitmaps[5] = kenan;

        for (int i = 0; i < bitmaps.length; i++) {
            Log.d("YuvPlayer","Bitmap:" + bitmaps[i]);
        }

//        draw3DCubeTexture(bitmaps,getHolder().getSurface(),surfaceWidth,surfaceHeight);
//        draw3DCubeWithColor(getHolder().getSurface(),surfaceWidth,surfaceHeight);

//        draw3DColorCubeCamera(getHolder().getSurface(),surfaceWidth,surfaceHeight);
        draw3DCubesCameraTouchCtl(getHolder().getSurface(),surfaceWidth,surfaceHeight);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int action = event.getAction() & MotionEvent.ACTION_MASK;
        handleTouchEvent(action,event.getX(),event.getY());
        return true;
    }


    public native void loadYuv(Object surface, AssetManager assetManager);

    public native void loadYuvWithFilterEffect(Object surface, AssetManager assetManager
            ,int filterType,boolean isNeedScaleAnim);

    public native void loadYuvWithBlurEffect(Object surface, AssetManager assetManager
            ,int filterType);

    public native void loadYuvWithSoulFled(Object surface, AssetManager assetManager);


    interface FilterType{
        /**
         * 没有滤镜
         */
        int NONE = 0;
        /**
         * 反色滤镜
         */
        int OPPO = 1;
        /**
         * 反色灰度图滤镜
         */
        int OPPO_GRAY = 2;
        /**
         * 灰度图滤镜
         */
        int GRAY = 3;
        /**
         * 二分镜
         */
        int DIVIDE_TO_2 = 4;
        /**
         * 四分镜
         */
        int DIVIDE_TO_4 = 5;
    }

    /**
     * 绘制三角形的native方法
     *
     * @param surface
     */
    public native void drawTriangle(Object surface);

    /**
     * 绘制2个三角形
     * @param surface
     */
    public native void drawTwoTriangle(Object surface);

    /**
     * 绘制点
     * @param surface
     */
    public native void drawPoints(Object surface);

    /**
     * 绘制线
     * @param surface
     */
    public native void drawLine(Object surface);

    /**
     * 绘制三角形，传递指定颜色
     * @param surface
     */
    public native void drawTriangleWithColorPass(Object surface);

    /**
     * 使用缓冲对象绘制三角形
     * @param surface
     */
    public native void drawTriangleWithBufferObj(Object surface);

    /**
     * 使用EBO绘制三角形
     * @param surface
     */
    public native void drawTriangleWithEBO(Object surface);

    /**
     * 使用Uniform变量
     * @param surface
     */
    public native void drawTriangleUniform(Object surface);

    /**
     * 指定颜色绘制线段
     * @param surface
     */
    public native void drawLineWithColor(Object surface);

    /**
     * 绘制纹理
     * @param bitmap
     * @param bitmap1
     * @param surface
     */
    public native void drawTexture(Bitmap bitmap, Bitmap bitmap1, Object surface);

    /**
     * 绘制3D纹理
     * @param bitmap
     * @param bitmap1
     * @param surface
     */
    public native void draw3DTexture(Bitmap bitmap, Bitmap bitmap1, Object surface,int screenWidth, int screenHeight);

    /**
     * 绘制立方体贴纹理
     * @param bitmapList
     * @param surface
     * @param surfaceWidth
     * @param surfaceHeight
     */
    public native void draw3DCubeTexture(Bitmap[] bitmapList, Surface surface, int surfaceWidth, int surfaceHeight);

    /**
     * 绘制渐变色立方体
     * @param bitmap
     * @param bitmap1
     * @param surface
     * @param screenWidth
     * @param screenHeight
     */
    public native void draw3DCubeWithColor(Object surface,int screenWidth, int screenHeight);


    /**
     * 绘制多个渐变色立方体并移动摄像机位置
     * @param surface
     * @param screenWidth
     * @param screenHeight
     */
    public native void draw3DColorCubeCamera(Object surface,int screenWidth, int screenHeight);

    /**
     * 绘制多个渐变色立方体，可触摸移动摄像机位置
     * @param surface
     * @param screenWidth
     * @param screenHeight
     */
    public native void draw3DCubesCameraTouchCtl(Object surface,int screenWidth, int screenHeight);

    public native void handleTouchEvent(int action,float x, float y);

//    public native void drawTexture(int[] bitmapArr, int w, int h, Object surface);




    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

    }

    private int surfaceWidth ;
    private int surfaceHeight ;

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        surfaceWidth = width;
        surfaceHeight = height;
    }

    @Override
    public void onDrawFrame(GL10 gl) {

    }

    private int[] getBitmapPixelArray(int resID) {
        Bitmap bitmap = ((BitmapDrawable) getResources().getDrawable(resID)).getBitmap();
        int w = bitmap.getWidth(), h = bitmap.getHeight();
        int[] pix = new int[w * h];
        bitmap.getPixels(pix, 0, w, 0, 0, w, h);
        return pix;
    }


}
