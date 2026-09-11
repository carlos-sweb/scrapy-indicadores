package com.carlossweb.indicadores

import android.content.Context
import com.lynx.tasm.provider.AbsTemplateProvider
import java.io.IOException

/**
 * Reads a Lynx bundle from app assets on a background thread — matching
 * lynx-family/lynx-examples's own explorer/android DemoTemplateProvider
 * pattern (loadTemplate() runs its I/O inside `Thread { ... }.start()`,
 * never on the caller's thread).
 *
 * Confirmed on real hardware 2026-09-10 this is not optional: reading the
 * same asset synchronously in MainActivity.onCreate() (blocking the Android
 * *UI* thread — separate from Lynx's own main/background thread split, but
 * blocking it just as effectively delays first paint) measured a 2.4-2.8s
 * cold start (`adb shell am start -W`), while LynxExplorer loading this
 * exact same bundle over HTTP — necessarily off the UI thread, since
 * DemoTemplateProvider only ever reads asynchronously — measured 292ms.
 * Switching to this async provider is what actually closes that gap, not
 * anything about main-thread vs background-thread choice inside the bundle
 * itself.
 */
class AssetTemplateProvider(private val context: Context) : AbsTemplateProvider() {
    override fun loadTemplate(url: String, callback: Callback) {
        Thread {
            try {
                val bytes = context.assets.open(url).use { it.readBytes() }
                callback.onSuccess(bytes)
            } catch (e: IOException) {
                callback.onFailed(e.toString())
            }
        }.start()
    }
}
