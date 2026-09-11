# indicadores-app

A one-page [Lynx](https://lynxjs.org) app built with [Mithril.js](https://mithril.js.org), via [`mithril-lynx`](https://github.com/carlos-sweb/mithril-lynx). Shows the daily Chilean economic indicators (UF, Dólar, Euro, Yen, Oro, Plata, Cobre) scraped from Banco Central de Chile by [`scrapy-indicadores`](https://github.com/carlos-sweb/scrapy-indicadores), which republishes them once a day at:

```
https://raw.githubusercontent.com/carlos-sweb/scrapy-indicadores/refs/heads/master/www-src/data.json
```

## Getting started

```bash
npm install
npm run dev      # scan the printed QR code with LynxExplorer
npm run build     # production bundle, in dist/
```

## How it's wired

This app uses `mithril-lynx`'s **data-channel mode**, not the simpler main-thread-owned mode most templates use — because `lynx.fetch()` only exists on the background/JS thread (there is no networking API on Lynx's main thread, and Mithril's own `m.request` can't fill the gap either — it's hard-wired to a real `XMLHttpRequest`, which doesn't exist in Lynx's runtime; see `mithril-lynx/README.md`'s "Known gap, not permanent" section):

- `src/background.ts` — sibling of `main-thread.ts`, auto-detected as a second bundle chunk by `mithril-lynx/plugin`. Fetches `data.json` with `lynx.fetch()` on load and on every `"refresh"` request from the UI, and pushes the result to the main thread with `setData()`.
- `src/main-thread.ts` — mounts the Mithril root once via `setupApp()`; every later `background.ts` push flows through the shim's own `redraw()`.
- `src/index.ts` — the UI: a title, a loading/error state, a card per indicator, and a "Refrescar" button that calls `dispatchToBackground("refresh")`.

## Learn more

Everything about the framework — the three rendering modes, data-channel mode, refs, gestures, navigation — is documented in [`mithril-lynx`'s own README](https://github.com/carlos-sweb/mithril-lynx#readme).
