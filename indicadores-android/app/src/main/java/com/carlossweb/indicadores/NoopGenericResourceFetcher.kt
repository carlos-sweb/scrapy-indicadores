package com.carlossweb.indicadores

import android.util.Base64
import com.lynx.tasm.resourceprovider.LynxResourceCallback
import com.lynx.tasm.resourceprovider.LynxResourceRequest
import com.lynx.tasm.resourceprovider.LynxResourceResponse
import com.lynx.tasm.resourceprovider.generic.LynxGenericResourceFetcher

/**
 * Decodes `data:` URIs for @font-face src resolution — nothing else, since
 * indicadores-app never references a real remote/asset resource this way.
 *
 * Root cause of the ~1.4-1.5s cold-start cost custom fonts added, found by
 * diffing LynxExplorer's own logcat while it loaded this exact bundle fast
 * (2026-09-10): it logs `FontFaceManager: Try to loadTypeface with
 * GenericLynxResourceFetcher` → success in ~13ms. @font-face src resolution
 * — even for a `data:` URI, not just real URLs — routes through
 * LynxGenericResourceFetcher.fetchResource(); WITHOUT one registered (or
 * with one that unconditionally fails, which a first attempt at this class
 * did), that fast path fails and the engine falls back to a much slower
 * legacy font-loading path, adding the fixed ~1.4-1.5s regardless of font
 * size — matching everything observed in that investigation.
 */
class NoopGenericResourceFetcher : LynxGenericResourceFetcher() {
    @Suppress("UNCHECKED_CAST")
    override fun fetchResource(request: LynxResourceRequest, callback: LynxResourceCallback<ByteArray>) {
        val url = request.url
        val commaIndex = url.indexOf(',')
        if (url.startsWith("data:") && commaIndex != -1) {
            try {
                val bytes = Base64.decode(url.substring(commaIndex + 1), Base64.DEFAULT)
                callback.onResponse(LynxResourceResponse.onSuccess(bytes))
                return
            } catch (e: IllegalArgumentException) {
                // fall through to failure below
            }
        }
        val response = LynxResourceResponse.onFailed(Throwable("not supported: $url")) as LynxResourceResponse<ByteArray>
        callback.onResponse(response)
    }

    @Suppress("UNCHECKED_CAST")
    override fun fetchResourcePath(request: LynxResourceRequest, callback: LynxResourceCallback<String>) {
        val response = LynxResourceResponse.onFailed(Throwable("not supported")) as LynxResourceResponse<String>
        callback.onResponse(response)
    }
}
