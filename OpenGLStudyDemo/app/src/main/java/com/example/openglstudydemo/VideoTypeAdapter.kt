package com.example.openglstudydemo


import android.content.Context
import android.content.Intent
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.annotation.NonNull
import androidx.recyclerview.widget.RecyclerView

/**
 * 视频类型列表Adapter
 */
class VideoTypeAdapter(val context: Context): RecyclerView.Adapter<VideoTypeAdapter.VideoTypeViewHolder>() {
    private val TAG = "VideoTypeAdapter"

    inner class VideoTypeViewHolder(itemView: View): RecyclerView.ViewHolder(itemView) {
        val textView:TextView? = itemView.findViewById<TextView>(R.id.tv_name)
        init{
            itemView.setOnClickListener {
                Intent().apply {
                    setClass(context,MainActivity::class.java)
                    putExtra(Config.VideoTypeKey, Config.videoTypeList[adapterPosition].type)
                    context.startActivity(this)
                }
            }
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): VideoTypeViewHolder {
        val v = LayoutInflater.from(parent.context)
            .inflate(R.layout.video_type_list_item, parent, false)
        Log.d(TAG, "onCreateViewHolder v:$v")
        return  VideoTypeViewHolder(v);
    }

    override fun getItemCount(): Int {
        Log.d(TAG, "getItemCount:${Config.videoTypeList.size}")
        return Config.videoTypeList.size
    }

    override fun onBindViewHolder(viewHolder: VideoTypeViewHolder, index: Int) {
        viewHolder.textView?.text = Config.videoTypeList[index].name
        Log.d(TAG, "onBindViewHolder")
    }
}