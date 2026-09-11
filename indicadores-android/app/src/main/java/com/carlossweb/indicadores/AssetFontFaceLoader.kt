package com.carlossweb.indicadores

import android.graphics.Typeface
import com.lynx.tasm.behavior.LynxContext
import com.lynx.tasm.fontface.FontFace
import com.lynx.tasm.loader.LynxFontFaceLoader

// Public, documented Lynx extension point (com.lynx.tasm.loader.LynxFontFaceLoader,
// shipped in the core `lynx` artifact — no extra dependency) for resolving custom
// @font-face src schemes. Without a registered Loader, Lynx's own default
// (LynxFontFaceLoader$1, decompiled from lynx-4.1.0.aar) is a no-op that never
// resolves "asset:///" — confirmed by inspecting FontFaceManager's bytecode:
// asset:/// is only handled inline inside loadTypeface() when a "FONT"
// LynxResourceProvider is registered (which we don't have), and prefetchFont()'s
// non-http/non-data:  branch (prefetchFontWithLoader) goes ONLY through this
// Loader, with no built-in asset:/// fallback of its own.
//
// This mirrors what "Indicadores Chile" (com.indicadores.chile, a competing
// Lynx/ReactLynx app analyzed 2026-09-10) does from its JS side via a NativeModule
// call it names `addFont` — same idea (resolve the font off the standard
// @font-face-triggered layout path), implemented here on the native side instead
// so MainActivity can also *prefetch* the exact same Typeface before
// renderTemplateUrl() ever runs — see MainActivity.kt's prefetchFont() call.
object AssetFontFaceLoader : LynxFontFaceLoader.Loader() {
    private const val ASSET_PREFIX = "asset:///"

    override fun onLoadFontFace(
        context: LynxContext,
        type: FontFace.TYPE,
        src: String,
    ): Typeface? {
        if (!src.startsWith(ASSET_PREFIX)) return null
        return try {
            Typeface.createFromAsset(context.context.assets, src.removePrefix(ASSET_PREFIX))
        } catch (e: Exception) {
            null
        }
    }
}
