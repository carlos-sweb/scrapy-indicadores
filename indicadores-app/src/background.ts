import { setupBackground, setData, setBackgroundEventHandler } from "mithril-lynx/background";

// Sibling of main-thread.ts — mithril-lynx/plugin picks this up automatically
// as the second (background/JS-thread) bundle chunk. Only this thread has
// lynx.fetch() (see @lynx-js/types/background-thread/fetch.d.ts); the main
// thread never touches the network directly.
const DATA_URL =
  "https://raw.githubusercontent.com/carlos-sweb/scrapy-indicadores/refs/heads/master/www-src/data.json";

// Lynx ships no built-in persistent storage (confirmed: nothing "Storage"-
// shaped anywhere in @lynx-js/types; lynx-examples/examples/local-storage's
// NativeModules.NativeLocalStorageModule is never actually implemented
// there either — every real usage, e.g. explorer/android's ExplorerModule,
// is a hand-written native module backed by SharedPreferences/UserDefaults/
// etc.). indicadores-android registers one of our own —
// IndicadoresStorageModule, a thin SharedPreferences wrapper — via
// LynxViewBuilder.registerModule() in MainActivity. It won't exist when
// this bundle runs in a host that never registered it (e.g. LynxExplorer),
// so every call below is optional-chained and never assumed to succeed.
const CACHE_KEY = "indicadores-cache";
const THEME_KEY = "theme";

interface Indicadores {
  UF?: string;
  Dolar?: string;
  Euro?: string;
  Yen?: string;
  Oro?: string;
  Plata?: string;
  Cobre?: string;
}

interface Cache {
  date: string; // YYYY-MM-DD, local date — see todayKey()
  indicadores: Indicadores;
  updatedAt: number;
}

// Banco Central publishes these once a day — re-fetching on every cold
// start is pure waste. Cache by calendar date; only a date mismatch (a new
// day) or an explicit "refresh" tap (dispatchToBackground("refresh") from
// the UI's Refrescar button) triggers a real network fetch.
function todayKey(): string {
  const d = new Date();
  const mm = String(d.getMonth() + 1).padStart(2, "0");
  const dd = String(d.getDate()).padStart(2, "0");
  return `${d.getFullYear()}-${mm}-${dd}`;
}

function readCache(): Cache | null {
  try {
    const raw = NativeModules.IndicadoresStorageModule?.get(CACHE_KEY);
    if (!raw) return null;
    return JSON.parse(raw) as Cache;
  } catch {
    return null;
  }
}

function writeCache(cache: Cache) {
  try {
    NativeModules.IndicadoresStorageModule?.set(CACHE_KEY, JSON.stringify(cache));
  } catch {
    // Best-effort — a cache write failure just means next cold start
    // re-fetches, not a user-visible error.
  }
}

async function loadIndicadores() {
  setData({ loading: true, error: undefined });
  try {
    const response = await lynx.fetch(DATA_URL);
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const indicadores = (await response.json()) as Indicadores;
    const updatedAt = Date.now();
    writeCache({ date: todayKey(), indicadores, updatedAt });
    setData({ indicadores, updatedAt, loading: false, error: undefined });
  } catch (err) {
    setData({
      loading: false,
      error: err instanceof Error ? err.message : "No se pudo cargar la información",
    });
  }
}

setupBackground();
setBackgroundEventHandler((handlerName, data) => {
  // The Refrescar button always means "fetch now, regardless of cache" —
  // an explicit user request overrides the date check below.
  if (handlerName === "refresh") loadIndicadores();
  // index.ts's ThemeToggle fires this on every tap, main thread already
  // redrawn — this just persists the choice for the *next* cold start.
  // MainActivity.kt reads this same key synchronously at launch and seeds
  // initData with it, so the app opens directly in the last theme instead
  // of always defaulting to dark.
  if (handlerName === "themeChanged" && (data === "dark" || data === "light")) {
    try {
      NativeModules.IndicadoresStorageModule?.set(THEME_KEY, data);
    } catch {
      // Best-effort — a failed write just means next cold start falls back
      // to dark, not a user-visible error.
    }
  }
});

const cached = readCache();
if (cached && cached.date === todayKey()) {
  setData({
    indicadores: cached.indicadores,
    updatedAt: cached.updatedAt,
    loading: false,
    error: undefined,
  });
} else {
  loadIndicadores();
}
