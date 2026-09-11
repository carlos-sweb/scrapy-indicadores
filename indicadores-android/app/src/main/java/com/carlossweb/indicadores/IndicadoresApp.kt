package com.carlossweb.indicadores

import android.app.Application
import com.lynx.service.http.LynxHttpService
import com.lynx.tasm.LynxEnv
import com.lynx.tasm.loader.LynxFontFaceLoader
import com.lynx.tasm.service.LynxServiceCenter

class IndicadoresApp : Application() {
    override fun onCreate() {
        super.onCreate()

        // The core `lynx` artifact only declares service *interfaces*; a
        // concrete implementation must be registered before use, or a
        // runtime call to that surface fails with "Lynx Http Service Not
        // registered" — confirmed on real hardware 2026-09-10 when
        // background.ts's lynx.fetch() call to scrapy-indicadores' data.json
        // hit exactly that error. Matches lynx-examples's own
        // ExplorerApplication.initLynxService() pattern.
        // Kotlin `object LynxHttpService : ILynxHttpService` is already the
        // singleton reference — no `.INSTANCE` needed from Kotlin (that's
        // the Java-interop static field the Explorer's own Java code uses).
        LynxServiceCenter.inst().registerService(LynxHttpService)

        // Tried and removed 2026-09-10: reflectively overriding Android's
        // Typeface.DEFAULT/sSystemFontMap before this init() call, hoping
        // Lynx's un-styled text falls back to Android's own default-font
        // machinery. It doesn't — Lynx renders text through its own native
        // font system, entirely independent of android.graphics.Typeface,
        // so the override was a confirmed no-op for LynxView content (it
        // patched successfully, text stayed on Lynx's own default font).
        // Real fix for the underlying cost is tracked upstream:
        // https://github.com/lynx-family/lynx/issues/9431

        // 2026-09-10: re-attempting the custom font, this time via
        // FontFaceManager.prefetchFont() (see MainActivity.kt) instead of a
        // synchronous @font-face resolution during the first layout. This
        // Loader is what lets "asset:///" resolve at all — see
        // AssetFontFaceLoader's own header comment.
        LynxFontFaceLoader.setLoader(AssetFontFaceLoader)

        LynxEnv.inst().init(this, null, null, null)
    }
}
