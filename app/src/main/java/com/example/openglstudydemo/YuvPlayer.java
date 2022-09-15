package com.example.openglstudydemo;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;

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
//        loadYuv(PATH,getHolder().getSurface());

//     drawTwoTriangle(getHolder().getSurface());
        drawTriangleWithEBO(getHolder().getSurface());
        Log.d("YuvPlayer", "loadYuv");
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

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {

    }

    @Override
    public void onDrawFrame(GL10 gl) {

    }
}
