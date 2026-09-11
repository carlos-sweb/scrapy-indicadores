package com.carlossweb.indicadores

import android.content.Context
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import com.lynx.tasm.LynxBooleanOption
import com.lynx.tasm.LynxViewBuilder
import com.lynx.tasm.ThreadStrategyForRendering
import com.lynx.tasm.fontface.FontFaceManager
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import org.json.JSONObject

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        // Must run before super.onCreate() per the SplashScreen API contract.
        // With no setKeepOnScreenCondition(...) set, the splash clears on its
        // own default signal — this Activity's first drawn frame — so it
        // covers exactly the real cold-start window (~700-800ms) rather than
        // an arbitrary fixed delay.
        installSplashScreen()
        super.onCreate(savedInstanceState)

        val builder = LynxViewBuilder()
        // Backs background.ts's day-based data.json cache — see
        // IndicadoresStorageModule's own header comment.
        builder.registerModule("IndicadoresStorageModule", IndicadoresStorageModule::class.java)
        // lynx-family/lynx's own explorer/android registers a
        // GenericResourceFetcher unconditionally (LynxViewShellActivity —
        // "used inside LynxEngine for resource loading capabilities of
        // components such as Text"); we didn't. Testing whether this is
        // what lets Explorer load a @font-face-heavy bundle cold in ~300ms
        // while ours took ~2.2-2.9s with the same font — see
        // NoopGenericResourceFetcher's own header comment.
        builder.setEnableGenericResourceFetcher(LynxBooleanOption.TRUE)
        builder.setGenericResourceFetcher(NoopGenericResourceFetcher())
        builder.setThreadStrategyForRendering(ThreadStrategyForRendering.ALL_ON_UI)
        // Reads dist/main-thread.bundle (copied into assets/ at build time)
        // off the Android UI thread — see AssetTemplateProvider's own header
        // comment for why this isn't optional: reading it synchronously
        // here, before Lynx gets a chance to render anything, measured a
        // 2.4-2.8s cold start on real hardware vs. 292ms once moved here.
        builder.setTemplateProvider(AssetTemplateProvider(this))
        val lynxView = builder.build(this)
        setContentView(lynxView)

        // Prefetch the custom font on Lynx's own IO thread pool as early as
        // possible — before renderTemplateUrl() below ever gives the JS
        // bundle's CSS a chance to trigger @font-face resolution during the
        // first layout. FontFaceManager caches by the exact "src" string
        // (see srcKey()/cachePrefetchedTypeface() in lynx-4.1.0.aar), so this
        // "asset:///fonts/ubuntu_mono.ttf" URI must match style.css's
        // @font-face src url() byte-for-byte for the later real lookup to
        // hit this warmed cache entry instead of resolving cold, synchronously,
        // inside __FlushElementTree() — see https://github.com/lynx-family/lynx/issues/9431.
        // Requires AssetFontFaceLoader (IndicadoresApp.kt) to be registered —
        // without it "asset:///" isn't resolvable by either this prefetch or
        // the real @font-face lookup.
        FontFaceManager.getInstance().prefetchFont(
            lynxView.lynxContext,
            "asset:///fonts/ubuntu_mono.ttf",
            null,
            object : FontFaceManager.FontFacePrefetchListener {
                override fun onComplete(code: Int, msg: String) {}
            },
        )

        // IFR (https://lynxjs.org/next/guide/interaction/ifr.html): read
        // both today's cached data.json AND the last saved theme
        // *synchronously* here, outside the JS engine entirely, and hand
        // them to Lynx as this page's initData — same pattern proven in
        // indicadores-react-android's MainActivity. main-thread.js's
        // setupApp() reads it straight into getData() on the very first
        // __RenderPage, before background.ts's own (async) NativeModules
        // reads even run — so index.ts's view() sees data.indicadores (on a
        // cache hit) and data.theme already populated on the first frame.
        // Without this, the app painted the skeleton first even when the
        // exact same cached JSON was one synchronous SharedPreferences read
        // away, and always opened in dark mode regardless of what the user
        // picked last time — both are what this fixes.
        //
        // This single bundle already contains both the main-thread (lepus)
        // and background (JS-thread) chunks, encoded together by
        // mithril-lynx/plugin's dual-bundle pipeline — one renderTemplateUrl
        // call loads both.
        val initData = readInitData()
        lynxView.renderTemplateUrl("main-thread.bundle", initData ?: "")
    }

    private fun readInitData(): String? {
        val prefs = getSharedPreferences("indicadores_storage", Context.MODE_PRIVATE)
        val initData = JSONObject()

        // Written by background.ts's setBackgroundEventHandler on every
        // ThemeToggle tap (see index.ts) — key/format must match exactly.
        try {
            val theme = prefs.getString("theme", null)
            if (theme == "dark" || theme == "light") {
                initData.put("theme", theme)
            }
        } catch (e: Exception) {
            // Best-effort — falls back to index.ts's own "dark" default.
        }

        // Written by background.ts's writeCache() after a successful fetch.
        try {
            val raw = prefs.getString("indicadores-cache", null)
            if (raw != null) {
                val cache = JSONObject(raw)
                val today = SimpleDateFormat("yyyy-MM-dd", Locale.US).format(Date())
                if (cache.optString("date") == today) {
                    initData.put("indicadores", cache.getJSONObject("indicadores"))
                    initData.put("updatedAt", cache.getLong("updatedAt"))
                }
            }
        } catch (e: Exception) {
            // Best-effort — background.ts fetches fresh data on a miss.
        }

        return if (initData.length() > 0) initData.toString() else null
    }
}
