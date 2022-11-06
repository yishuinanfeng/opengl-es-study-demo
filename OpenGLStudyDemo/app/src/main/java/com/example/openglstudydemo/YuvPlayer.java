package com.example.openglstudydemo;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;

import java.util.Arrays;

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

    }

    @Override
    public void run() {
        Log.d("YuvPlayer", "run");

        AssetManager assetManager = getContext().getAssets();
//        loadYuv(getHolder().getSurface(),assetManager);
        loadYuvWithFilterEffect(getHolder().getSurface(),assetManager,FilterType.GRAY);



//     drawTwoTriangle(getHolder().getSurface());
//        drawTriangle(getHolder().getSurface());
//        drawLineWithColor(getHolder().getSurface());
//        drawTriangleUniform(getHolder().getSurface());
//        drawTriangleWithEBO(getHolder().getSurface());
//        drawTriangleWithBufferObj(getHolder().getSurface());

//        drawableTexture();

        Log.d("YuvPlayer", "loadYuv");
    }

    private void drawableTexture() {
        Bitmap bitmap = ((BitmapDrawable) getResources().getDrawable(R.drawable.shiyuanmeili2)).getBitmap();
        Bitmap bitmap1 = ((BitmapDrawable) getResources().getDrawable(R.drawable.liyingai)).getBitmap();

//        BitmapFactory.Options options1 = new BitmapFactory.Options();
//        options1.inPreferredConfig = Bitmap.Config.RGB_565;
//        Bitmap bitmap = BitmapFactory.decodeResource(getResources(),R.drawable.animal,options1);


        int w = bitmap.getWidth(), h = bitmap.getHeight();
        int[] pix = new int[w * h];
        bitmap.getPixels(pix, 0, w, 0, 0, w, h);

        Log.d("YuvPlayer", "drawableTexture:" + "w:" + w+ ",h:" + h + "，pix:" + Arrays.toString(pix));


//        drawTexture(bitmap, w, h, getHolder().getSurface());
        drawTexture(bitmap,bitmap1,getHolder().getSurface());
    }

    public native void loadYuv(Object surface, AssetManager assetManager);

    public native void loadYuvWithFilterEffect(Object surface, AssetManager assetManager
            ,int filterType);


    interface FilterType{
        int GRAY = 0;
        int OPPO = 1;

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

//    public native void drawTexture(int[] bitmapArr, int w, int h, Object surface);


    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {

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
