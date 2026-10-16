package com.platinumfast

import android.content.Context
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.SurfaceHolder
import android.view.SurfaceView

// Dual-screen placeholder: top screen = video out (device pass renders here),
// bottom screen = touch input mapped to NDS 256x192 resistive coords.
// v0.3-shell: draws nothing, only forwards touches; renderer plugs in later.
class EmuView @JvmOverloads constructor(
  ctx: Context, attrs: AttributeSet? = null
) : SurfaceView(ctx, attrs), SurfaceHolder.Callback {

  var onTouchNds: ((x: Int, y: Int, down: Boolean) -> Unit)? = null

  init { holder.addCallback(this) }

  override fun surfaceCreated(h: SurfaceHolder) {}
  override fun surfaceChanged(h: SurfaceHolder, f: Int, w: Int, h2: Int) {}
  override fun surfaceDestroyed(h: SurfaceHolder) {}

  override fun onTouchEvent(e: MotionEvent): Boolean {
    // Bottom half = touchscreen. Map view px -> 0..255 x 0..191.
    val isBottom = e.y > height / 2f
    if (!isBottom) return false
    val nx = ((e.x / width) * 256).toInt().coerceIn(0, 255)
    val ny = (((e.y - height / 2f) / (height / 2f)) * 192).toInt().coerceIn(0, 191)
    when (e.action) {
      MotionEvent.ACTION_DOWN -> onTouchNds?.invoke(nx, ny, true)
      MotionEvent.ACTION_MOVE -> onTouchNds?.invoke(nx, ny, true)
      MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> onTouchNds?.invoke(nx, ny, false)
    }
    return true
  }
}
