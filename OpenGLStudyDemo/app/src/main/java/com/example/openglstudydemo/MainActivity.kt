package com.example.openglstudydemo

import android.graphics.drawable.BitmapDrawable
import android.os.Bundle
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
    // Used to load the 'native-lib' library on application startup.
    private var videoType = VideoTypeEnum.SIMPLE_YUV
    private val yuvPlayer by lazy<YuvPlayer> {
        findViewById(R.id.yuv_player)
    }
    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
//        val bitmap = (resources.getDrawable(R.drawable.liyingai1) as BitmapDrawable).bitmap
//        findViewById<ImageView>(R.id.img).setImageBitmap(bitmap)
        // Example of a call to a native method
//        TextView tv = findViewById(R.id.sample_text);
//        tv.setText(stringFromJNI());

        intent?.let {
            videoType = (intent.getSerializableExtra(Config.VideoTypeKey) as? VideoTypeEnum)
                ?:VideoTypeEnum.SIMPLE_YUV
            yuvPlayer.setVideoType(videoType)
        }
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String?

}