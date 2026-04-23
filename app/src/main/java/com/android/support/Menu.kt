package com.android.support

import android.app.Activity
import android.content.Context
import android.content.SharedPreferences
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Color
import android.graphics.PixelFormat
import android.graphics.Typeface
import android.graphics.drawable.ShapeDrawable
import android.text.TextUtils
import android.util.Base64
import android.util.TypedValue
import android.view.Gravity
import android.view.MotionEvent
import android.view.View
import android.view.View.OnTouchListener
import android.view.WindowManager
import android.widget.Button
import android.widget.FrameLayout
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.RelativeLayout
import android.widget.ScrollView
import android.widget.SeekBar
import android.widget.Switch
import android.widget.TextView
import android.widget.Toast
import androidx.core.content.edit
import androidx.core.graphics.toColorInt
import androidx.core.view.setPadding

// setup

const val ICON_SIZE = 100
const val POS_X = 20
const val POS_Y = 50

const val MENU_WIDTH = 290

val MENU_BG_COLOR = "#EE1C2A35".toColorInt()//#AARRGGBB
val MENU_FEATURE_BG_COLOR = "#DD141C22".toColorInt() //#AARRGGBB

val TEXT_COLOR = "#82CAFD".toColorInt()
val TEXT_COLOR_2 = "#FFFFFF".toColorInt()
val CATEGORY_BG_COLOR = "#2F3D4C".toColorInt()


class Menu(val context: Activity) {

    val rootFrame: FrameLayout = FrameLayout(context)
    val menuContainer: LinearLayout = LinearLayout(context)
    val startIcon: ImageView = ImageView(context)

    val sharedPreferences: SharedPreferences = context.getSharedPreferences(
        context.packageName + "_preferences",
        Context.MODE_PRIVATE
    )
    val onTouchListener = object : OnTouchListener {
        var dx: Float = 0f
        var dy: Float = 0f
        var isClicked = true
        override fun onTouch(v: View, event: MotionEvent): Boolean {
            when (event.action) {
                MotionEvent.ACTION_DOWN -> {
                    var params = rootFrame.layoutParams as WindowManager.LayoutParams
                    dx = params.x - event.rawX
                    dy = params.y - event.rawY
                    isClicked = true
                }

                MotionEvent.ACTION_MOVE -> {
                    isClicked = false
                    rootFrame.alpha = 0.5f
                    var params = rootFrame.layoutParams as WindowManager.LayoutParams
                    params.x = (event.rawX + dx).toInt()
                    params.y = (event.rawY + dy).toInt()
                    val windowManager = context.windowManager
                    var bounds = windowManager.currentWindowMetrics.bounds
                    if (params.x < POS_X) params.x = POS_X
                    else if (params.x > bounds.right - rootFrame.width - POS_X) {
                        params.x = bounds.right - rootFrame.width - POS_X
                    }
                    if (params.y < POS_Y) params.y = POS_Y
                    else if (params.y > bounds.bottom - rootFrame.height - POS_Y) {
                        params.y = bounds.bottom - rootFrame.height - POS_Y
                    }
                    windowManager.updateViewLayout(rootFrame, params)
                }

                MotionEvent.ACTION_UP -> {
                    var params = rootFrame.layoutParams as WindowManager.LayoutParams
                    write("POS_X", params.x)
                    write("POS_Y", params.y)
                    rootFrame.alpha = 1f
                    if (isClicked) {
                        v.performClick()
                    }
                }

                else -> return false
            }
            return true
        }
    }

    fun start() {
        initRootFrame()
        initStartIcon()
        initRootContainer()
        Toast.makeText(context, getTitle(), Toast.LENGTH_LONG).show()
    }

    fun initRootFrame() {
        rootFrame.setOnTouchListener(onTouchListener)
        val frameParams = WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.WRAP_CONTENT,
            read("POS_X", POS_X),  //initialX
            read("POS_Y", POS_Y),  //initialY
            WindowManager.LayoutParams.TYPE_APPLICATION,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE or
                    WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN or
                    WindowManager.LayoutParams.FLAG_SPLIT_TOUCH,
            PixelFormat.TRANSPARENT
        )
        frameParams.gravity = Gravity.TOP or Gravity.LEFT
        context.windowManager.addView(rootFrame, frameParams)
    }

    fun initStartIcon() {
        startIcon.layoutParams = RelativeLayout.LayoutParams(
            ICON_SIZE,
            ICON_SIZE,
        )
        startIcon.scaleType = ImageView.ScaleType.FIT_CENTER
        startIcon.setImageBitmap(decodeImage(getStartIcon()))
        startIcon.setOnTouchListener(onTouchListener)
        startIcon.setOnClickListener {
            startIcon.visibility = View.GONE
            menuContainer.visibility = View.VISIBLE
        }

        rootFrame.addView(startIcon)
    }

    fun initRootContainer() {
        menuContainer.visibility = View.GONE
        menuContainer.setBackgroundColor(MENU_BG_COLOR)
        menuContainer.orientation = LinearLayout.VERTICAL

        menuContainer.setLayoutParams(
            LinearLayout.LayoutParams(
                dp(MENU_WIDTH),
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
        )

        val head = LinearLayout(context)
        head.orientation = LinearLayout.VERTICAL
        head.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )
        val divider = ShapeDrawable()
        divider.intrinsicHeight = 10
        divider.alpha = 0
        head.setDividerDrawable(divider)
        head.showDividers = LinearLayout.SHOW_DIVIDER_MIDDLE
        head.setPadding(0, 20, 0, 20)
        menuContainer.addView(head)

        //********** Title **********
        val title = TextView(context)
        title.setTextColor(TEXT_COLOR)
        title.text = getTitle()
        title.textSize = 18.0f
        title.typeface = Typeface.DEFAULT_BOLD
        title.setGravity(Gravity.CENTER)
        title.layoutParams = RelativeLayout.LayoutParams(
            RelativeLayout.LayoutParams.MATCH_PARENT,
            RelativeLayout.LayoutParams.WRAP_CONTENT
        )
        head.addView(title)


        //********** Sub title **********
        val subTitle = TextView(context)
        subTitle.text = getSubTitle()
        subTitle.ellipsize = TextUtils.TruncateAt.MARQUEE
        subTitle.marqueeRepeatLimit = -1
        subTitle.setSingleLine(true)
        subTitle.setSelected(true)
        subTitle.setTextColor(TEXT_COLOR)
        subTitle.textSize = 12.0f
        subTitle.setGravity(Gravity.CENTER)
        head.addView(subTitle)

        menuContainer.addView(generateFeatureList())

        val actionBar = LinearLayout(context)
        actionBar.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )
        actionBar.gravity = Gravity.RIGHT

        val closeBtn = Button(context)
        closeBtn.text = "CLOSE"
        closeBtn.setBackgroundColor(Color.TRANSPARENT)
        closeBtn.setTextColor(TEXT_COLOR)
        closeBtn.setOnClickListener {
            menuContainer.visibility = View.GONE
            startIcon.visibility = View.VISIBLE
        }
        closeBtn.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.WRAP_CONTENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )

        actionBar.addView(closeBtn)

        menuContainer.addView(actionBar)
        rootFrame.addView(menuContainer)
    }

    fun decodeImage(base64String: String): Bitmap {
        val imageBytes = Base64.decode(base64String.substringAfter("base64,"), Base64.DEFAULT)
        return BitmapFactory.decodeByteArray(imageBytes, 0, imageBytes.size)
    }

    fun generateFeatureList(): View {
        //********** Mod menu feature list **********
        val scrollView = ScrollView(context)
        //Auto size. To set size manually, change the width and height example 500, 500
        val layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT,
        )
        scrollView.layoutParams = layoutParams
        scrollView.setBackgroundColor(MENU_FEATURE_BG_COLOR)
        scrollView.setPadding(0, 10, 0, 10)
        val mods = LinearLayout(context)
        mods.orientation = LinearLayout.VERTICAL
        val divider = ShapeDrawable()
        divider.intrinsicHeight = 10
        divider.alpha = 0
        mods.setDividerDrawable(divider)
        mods.showDividers = LinearLayout.SHOW_DIVIDER_MIDDLE
        val featureList = getFeatureList()
        if (featureList != null) {
            for ((idx, feat) in featureList.withIndex()) {
                feat.trim().split(":").let { it ->
                    when (it[0].lowercase()) {
                        "category" -> mods.addView(this.category(it[1]))
                        "toggle" -> {
                            var defaultValue = false
                            if (it.size >= 3) {
                                defaultValue = it[2].toBoolean()
                            }
                            mods.addView(this.toggle(idx, it[1], defaultValue))

                        }

                        "seekbar" -> {
                            var min = 1
                            var max = 10
                            var defaultValue = min
                            if (it.size >= 3) {
                                it[2].split("_").let {
                                    if (it.isNotEmpty()) {
                                        min = it[0].toInt()
                                        defaultValue = min
                                    }
                                    if (it.size >= 2) {
                                        max = it[1].toInt()
                                    }
                                    if (it.size >= 3) {
                                        defaultValue = it[2].toInt()
                                    }
                                }

                            }
                            mods.addView(this.seekbar(idx, it[1], min, max, defaultValue))
                        }
                    }

                }
            }
        }
        scrollView.addView(mods)
        return scrollView
    }

    external fun getTitle(): String
    external fun getSubTitle(): String
    external fun getStartIcon(): String

    external fun getFeatureList(): Array<String>?
    external fun valueChange(featIdx: Int, featName: String, value: Int)

    fun valueChange(featIdx: Int, featName: String, value: Boolean) {
        valueChange(featIdx, featName, if (value) 1 else 0)
    }

    @Suppress("UNCHECKED_CAST")
    fun <T> read(key: String, defaultValue: T): T {
        return when (defaultValue) {
            is String -> sharedPreferences.getString(key, defaultValue)
            is Int -> sharedPreferences.getInt(key, defaultValue)
            is Boolean -> sharedPreferences.getBoolean(key, defaultValue)
            else -> defaultValue
        } as T
    }

    fun write(key: String, value: Any) {
        when (value) {
            is String -> sharedPreferences.edit { putString(key, value) }
            is Int -> sharedPreferences.edit { putInt(key, value) }
            is Boolean -> sharedPreferences.edit { putBoolean(key, value) }
            else -> println("The value is of an unknown type.")
        }
    }


    private fun category(name: String): View {
        val textView = TextView(context)
        textView.setBackgroundColor(CATEGORY_BG_COLOR)
        textView.text = name
        textView.setGravity(Gravity.CENTER)
        textView.setTextColor(TEXT_COLOR_2)
        textView.typeface = Typeface.DEFAULT_BOLD
        textView.setPadding(10)
        return textView
    }

    private fun toggle(featIdx: Int, featName: String, defaultValue: Boolean = false): View {
        val isChecked = read(featName, defaultValue)
        valueChange(featIdx, featName, isChecked)
        val switch = Switch(context)
        switch.text = featName
        switch.isChecked = isChecked
        switch.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )
        switch.setTextColor(TEXT_COLOR_2)
        switch.setPadding(20)
        switch.setOnCheckedChangeListener { _, isChecked ->
            write(featName, isChecked)
            valueChange(featIdx, featName, isChecked)
        }
        return switch
    }

    private fun seekbar(
        featIdx: Int,
        featName: String,
        min: Int,
        max: Int,
        defaultValue: Int = min
    ): View {
        val process = read(featName, defaultValue)
        valueChange(featIdx, featName, process)

        val linearLayout = LinearLayout(context)
        linearLayout.orientation = LinearLayout.VERTICAL
        linearLayout.setPadding(20)
        val divider = ShapeDrawable()
        divider.intrinsicHeight = 20
        divider.alpha = 0
        linearLayout.setDividerDrawable(divider)
        linearLayout.showDividers = LinearLayout.SHOW_DIVIDER_MIDDLE

        val head = LinearLayout(context)
        head.orientation = LinearLayout.HORIZONTAL
        head.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )

        val name = TextView(context)
        name.text = featName
        name.setTextColor(TEXT_COLOR_2)
        name.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.WRAP_CONTENT,
            LinearLayout.LayoutParams.WRAP_CONTENT,
            1.0f
        )
        head.addView(name)

        val value = TextView(context)
        value.setTextColor(TEXT_COLOR)
        value.text = process.toString()
        value.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.WRAP_CONTENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )
        value.setPadding(20, 0, 20, 0)
        head.addView(value)

        linearLayout.addView(head)

        val seekbar = SeekBar(context)
        seekbar.min = min
        seekbar.max = max
        seekbar.progress = process
        seekbar.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT,
        )
        seekbar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                value.text = progress.toString()
                write(featName, progress)
                valueChange(featIdx, featName, progress)
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
            }
        })
        linearLayout.addView(seekbar)
        return linearLayout
    }


    private fun dp(i: Int): Int {
        return TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_DIP,
            i.toFloat(),
            context.resources.displayMetrics
        ).toInt()
    }
}



