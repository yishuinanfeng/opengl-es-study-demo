package com.example.openglstudydemo

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView

class EnterActivity : AppCompatActivity() {
    private val TAG = "EnterActivity"
//    private val rvVideoType: RecyclerView  by lazy{
//        findViewById(R.id.rv_video_type)
//    }

    private var videoTypeAdapter:VideoTypeAdapter? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
//        enableEdgeToEdge()
        setContentView(R.layout.activity_enter)
//        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main)) { v, insets ->
//            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
//            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
//            insets
//        }
        val rvVideoType = findViewById<RecyclerView>(R.id.rv_video_type)
        Log.d(TAG, "onCreate rvVideoType:$rvVideoType")
        rvVideoType.apply {
            layoutManager = LinearLayoutManager(this.context)
            videoTypeAdapter = VideoTypeAdapter(this@EnterActivity)
            adapter = videoTypeAdapter
            Log.d(TAG,"init rvVideoType")
        }


    }

    override fun onResume() {
        super.onResume()
//        videoTypeAdapter?.notifyDataSetChanged()
    }
}