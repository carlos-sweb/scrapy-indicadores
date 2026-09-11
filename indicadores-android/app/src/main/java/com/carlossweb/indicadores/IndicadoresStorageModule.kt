package com.carlossweb.indicadores

import android.content.Context
import com.lynx.jsbridge.LynxMethod
import com.lynx.jsbridge.LynxModule

/**
 * A minimal persistent key-value store for the background bundle
 * (indicadores-app/src/background.ts), backed by SharedPreferences.
 *
 * Lynx ships no built-in persistent storage — confirmed by grepping
 * @lynx-js/types for anything "Storage"-shaped (nothing) and checking
 * lynx-examples/examples/local-storage's NativeModules.NativeLocalStorageModule
 * (never actually implemented anywhere in that repo either). Every real
 * usage — e.g. lynx-family/lynx's own explorer/android ExplorerModule
 * (saveToLocalStorage/readFromLocalStorage) — is a hand-written native
 * module exactly like this one.
 *
 * Registered on the LynxView in MainActivity via
 * LynxViewBuilder.registerModule(...), so it's reachable from JS as
 * `NativeModules.IndicadoresStorageModule.get(key)` /
 * `.set(key, value)` — both synchronous, matching ExplorerModule's own
 * readFromLocalStorage()'s direct-return shape rather than a callback.
 */
class IndicadoresStorageModule(context: Context) : LynxModule(context) {
    private val prefs = context.getSharedPreferences("indicadores_storage", Context.MODE_PRIVATE)

    @LynxMethod
    fun get(key: String): String? = prefs.getString(key, null)

    @LynxMethod
    fun set(key: String, value: String) {
        prefs.edit().putString(key, value).apply()
    }
}
