package com.example.openglstudydemo;

import android.content.Context;
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
        loadYuv(PATH,getHolder().getSurface());

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

    public native void loadYuv(String url, Object surface);

    /**
     * 绘制三角形的native方法
     *
     * @param surface
     */
    public native void drawTriangle(Object surface);

    public native void drawTwoTriangle(Object surface);

    public native void drawPoints(Object surface);

    public native void drawLine(Object surface);

    public native void drawTriangleWithColorPass(Object surface);

    public native void drawTriangleWithBufferObj(Object surface);

    public native void drawTriangleWithEBO(Object surface);

    public native void drawTriangleUniform(Object surface);


    public native void drawLineWithColor(Object surface);

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
