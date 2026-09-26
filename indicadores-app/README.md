# indicadores-app

A one-page [Lynx](https://lynxjs.org) app built with [Mithril.js](https://mithril.js.org), via [`mithril-lynx`](https://github.com/carlos-sweb/mithril-lynx). Shows the daily Chilean economic indicators (UF, Dólar, Euro, Yen, Oro, Plata, Cobre) scraped from Banco Central de Chile by [`scrapy-indicadores`](https://github.com/carlos-sweb/scrapy-indicadores), which republishes them once a day at:

```
https://raw.githubusercontent.com/carlos-sweb/scrapy-indicadores/refs/heads/master/www-src/data.json
```

## Getting started

```bash
npm install
npm run dev               # scan the printed QR code with LynxExplorer
npm run build             # production bundle, in dist/
npm run android           # build + copy into ../indicadores-android + install and launch (debug)
npm run android:release   # build + copy + signed release APK
```

`npm run android*` is `scripts/android.mjs`, adapted from `create-mithril-lynx`'s template: it copies `dist/main-thread.bundle` and `dist/static/` (the font) into `../indicadores-android/app/src/main/assets/`. Release signing needs `../indicadores-android/keystore.properties` (gitignored) pointing at the app's existing release keystore — an update must be signed with the same key as the published APK, so never generate a new one.

## How it's wired

Built on `mithril-lynx` 3.x. Only the background (JS) thread runs app code; the main thread just replays the patches (see `mithril-lynx`'s README):

- `src/main-thread.ts` — `setupRenderer()`, nothing app-specific.
- `src/background.ts` — `renderApp({ root })` with the app from `index.ts`.
- `src/index.ts` — the whole app: state, the `data.json` fetch through `mithril-lynx/request`, a day-based cache and the theme in `IndicadoresStorageModule` (a SharedPreferences NativeModule registered by the Android host), and the view (a card per indicator, a "Refrescar" button, the light/dark toggle).
- The Ubuntu Sans Mono font is imported by `index.ts` and registered with `lynx.addFont()`; `output.assetPrefix: "asset:///"` (`lynx.config.ts`) makes the production bundle load it from the APK's assets, resolved by the host's `AssetFontFaceLoader`.

## Learn more

Everything about the framework is documented in [`mithril-lynx`'s own README](https://github.com/carlos-sweb/mithril-lynx#readme).
