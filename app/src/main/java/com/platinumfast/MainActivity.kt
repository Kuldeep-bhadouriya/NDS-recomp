package com.platinumfast

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.os.PowerManager
import android.widget.Button
import android.widget.LinearLayout
import android.widget.TextView
import android.widget.Toast
import java.io.File
import java.util.concurrent.atomic.AtomicBoolean
import kotlin.concurrent.thread

// v0.3-shell: SAF ROM pick -> copy to app files -> loadRom -> frame thread.
// Frame thread runs stepFrame() at 60Hz (or uncapped for FF); real vsync +
// Oboe audio + GLES view land in the device pass. No ROM bundled.
class MainActivity : Activity() {

  private val PICK_ROM = 1001
  private val running = AtomicBoolean(false)
  private var emuThread: Thread? = null
  private var ffMode = 0
  private lateinit var status: TextView
  private lateinit var emuView: EmuView

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    val layout = LinearLayout(this).apply { orientation = LinearLayout.VERTICAL }
    status = TextView(this).apply { text = "Pick your Platinum ROM (CPUE)" }
    layout.addView(status)
    emuView = EmuView(this).apply { minimumHeight = 800 }
    layout.addView(emuView, LinearLayout.LayoutParams(-1, 0, 1f))

    val pick = Button(this).apply { text = "Pick ROM" }
    pick.setOnClickListener {
      startActivityForResult(
        Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
          addCategory(Intent.CATEGORY_OPENABLE); type = "*/*"
        }, PICK_ROM)
    }
    layout.addView(pick)

    val ff = Button(this).apply { text = "FF: OFF" }
    ff.setOnClickListener {
      ffMode = when (ffMode) { 0 -> 2; 2 -> 4; 4 -> 255; else -> 0 }
      CoreBridge.setFastForward(ffMode)
      ff.text = "FF: " + when (ffMode) { 2 -> "2X"; 4 -> "4X"; 255 -> "UNCAPPED"; else -> "OFF" }
    }
    layout.addView(ff)

    val slots = LinearLayout(this).apply { orientation = LinearLayout.HORIZONTAL }
    for (s in 0..3) {
      val b = Button(this).apply { text = "S$s" }
      b.setOnClickListener {
        Toast.makeText(this, if (CoreBridge.saveState(s)) "Saved $s" else "Save $s failed",
          Toast.LENGTH_SHORT).show()
      }
      b.setOnLongClickListener {
        Toast.makeText(this, if (CoreBridge.loadState(s)) "Loaded $s" else "Load $s failed",
          Toast.LENGTH_SHORT).show(); true
      }
      slots.addView(b, LinearLayout.LayoutParams(0, -2, 1f))
    }
    layout.addView(slots)
    setContentView(layout)

    // Thermal headroom -> FF governor (uncapped sustain on SD678).
    val pm = getSystemService(POWER_SERVICE) as PowerManager
    thread(isDaemon = true) {
      while (true) {
        Thread.sleep(5000)
        if (!running.get()) continue
        val st = pm.currentThermalStatus
        CoreBridge.setThermalLevel(when {
          st >= PowerManager.THERMAL_STATUS_SEVERE -> 2
          st >= PowerManager.THERMAL_STATUS_MODERATE -> 1
          else -> 0
        })
      }
    }
  }

  override fun onActivityResult(req: Int, res: Int, data: Intent?) {
    super.onActivityResult(req, res, data)
    if (req != PICK_ROM || res != RESULT_OK) return
    val uri: Uri = data?.data ?: return
    try {
      val dst = File(filesDir, "rom.nds")
      contentResolver.openInputStream(uri)?.use { ins ->
        dst.outputStream().use { ins.copyTo(it) }
      }
      if (!CoreBridge.loadRom(dst.absolutePath)) {
        status.text = "loadRom failed (need CPUE dump)"
        return
      }
      status.text = "Running (stub core — interpreter lands next)"
      startLoop()
    } catch (t: Throwable) {
      status.text = "Pick failed: ${t.message}"
    }
  }

  private fun startLoop() {
    if (running.getAndSet(true)) return
    emuThread = thread {
      var frames = 0L
      var t0 = System.nanoTime()
      while (running.get()) {
        CoreBridge.stepFrame(false)
        frames++
        if (ffMode == 0) {
          // ~60Hz pacing until Choreographer vsync lands.
          try { Thread.sleep(16) } catch (_: InterruptedException) { break }
        }
        if (frames % 600 == 0L) {
          val dt = (System.nanoTime() - t0) / 1e9
          val fps = 600 / dt
          runOnUiThread { status.text = "frames=$frames fps=%.1f ff=$ffMode".format(fps) }
          t0 = System.nanoTime()
        }
      }
    }
  }

  override fun onDestroy() {
    running.set(false)
    try { CoreBridge.unload() } catch (_: Throwable) {}
    super.onDestroy()
  }
}
