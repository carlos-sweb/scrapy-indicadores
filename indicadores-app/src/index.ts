import m from "mithril-runtime";
import { redraw } from "mithril-lynx/mount-redraw";
import request from "mithril-lynx/request";

import ubuntuMonoRegular from "./assets/fonts/UbuntuSansMono-Regular.ttf";

// Cargas la fuente en runtime
lynx.addFont({
  'font-family': 'Ubuntu Mono Regular',
  src: `url("${ubuntuMonoRegular}")`,
}, () => { });

import "./style.css";

// v2 architecture note: main-thread.ts/background.ts are no longer two
// separate execution contexts for APP CODE — only main-thread.ts (Lepus VM)
// vs. this file (background/JS thread, the only one that ever runs a
// view() — see background.ts's own header) is a real thread split. There's
// no more getData()/dispatchToBackground() data-channel: state, the fetch,
// and the view all live here in one place, and a tap handler can mutate
// state and call NativeModules directly, no cross-thread dispatch needed.
//
// This also retires the old native-side IFR (initData) seeding this app
// used to need (see indicadores-android's MainActivity.kt readInitData()
// comment): readTheme()/readCache() below already run synchronously at
// module load, before renderApp()'s first performRender() — so the first
// frame already has the right theme and any same-day cached data, with no
// native-side plumbing required to achieve it.

interface Indicadores {
  UF?: string;
  Dolar?: string;
  Euro?: string;
  Yen?: string;
  Oro?: string;
  Plata?: string;
  Cobre?: string;
}

interface IndicadorDef {
  key: keyof Indicadores;
  label: string;
  unit: string;
}

const INDICADORES: IndicadorDef[] = [
  { key: "UF", label: "UF", unit: "CLP" },
  { key: "Dolar", label: "Dólar", unit: "CLP" },
  { key: "Euro", label: "Euro", unit: "CLP" },
  { key: "Yen", label: "Yen", unit: "CLP" },
  { key: "Oro", label: "Oro", unit: "USD / oz" },
  { key: "Plata", label: "Plata", unit: "USD / oz" },
  { key: "Cobre", label: "Cobre", unit: "USD / lb" },
];

// Only this thread has lynx.fetch() (see mithril-lynx/request's own
// implementation) — mithril-lynx/request wraps it with the real m.request
// shape (params/body/headers/timeout/error-shape), see REQUEST.md.
const DATA_URL =
  "https://raw.githubusercontent.com/carlos-sweb/scrapy-indicadores/refs/heads/master/www-src/data.json";

// Lynx ships no built-in persistent storage — indicadores-android registers
// its own NativeModule, IndicadoresStorageModule, a thin SharedPreferences
// wrapper (see MainActivity.kt). It won't exist in a host that never
// registered it (e.g. LynxExplorer), so every call below is optional-
// chained and never assumed to succeed.
const CACHE_KEY = "indicadores-cache";
const THEME_KEY = "theme";

interface Cache {
  date: string; // YYYY-MM-DD, local date — see todayKey()
  indicadores: Indicadores;
  updatedAt: number;
}

type Theme = "dark" | "light";

interface State {
  indicadores?: Indicadores;
  updatedAt?: number;
  loading: boolean;
  error?: string;
  theme: Theme;
}

// Banco Central publishes these once a day — re-fetching on every cold
// start is pure waste. Cache by calendar date; only a date mismatch (a new
// day) or an explicit "refresh" tap triggers a real network fetch.
function todayKey(): string {
  const d = new Date();
  const mm = String(d.getMonth() + 1).padStart(2, "0");
  const dd = String(d.getDate()).padStart(2, "0");
  return `${d.getFullYear()}-${mm}-${dd}`;
}

function readTheme(): Theme {
  try {
    const raw = NativeModules.IndicadoresStorageModule?.get(THEME_KEY);
    return raw === "light" ? "light" : "dark";
  } catch {
    return "dark";
  }
}

function persistTheme(theme: Theme) {
  try {
    NativeModules.IndicadoresStorageModule?.set(THEME_KEY, theme);
  } catch {
    // Best-effort — a failed write just means next cold start falls back
    // to readTheme()'s own "dark" default.
  }
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

console.log("[timing] module top-level start", Date.now());
const state: State = {
  loading: false,
  theme: readTheme(),
};
console.log("[timing] state initialized (theme read)", Date.now());

// mithril-lynx/request auto-redraws once the request settles (see
// REQUEST.md) — but that redraw is scheduled and can fire BEFORE this
// function's own .then()/.catch() body (the code that actually stores the
// response into `state`) runs, per REQUEST.md's own "Lynx timer quirk"
// section. `background: true` opts out of request()'s own redraw so the
// explicit redraw() calls below — which run only after `state` is already
// updated — are the ones that actually reach the screen.
async function loadIndicadores() {
  state.loading = true;
  state.error = undefined;
  redraw();
  try {
    const indicadores = await request<Indicadores>(DATA_URL, { background: true });
    const updatedAt = Date.now();
    writeCache({ date: todayKey(), indicadores, updatedAt });
    state.indicadores = indicadores;
    state.updatedAt = updatedAt;
    state.loading = false;
    redraw();
  } catch (err) {
    state.loading = false;
    state.error = err instanceof Error ? err.message : "No se pudo cargar la información";
    redraw();
  }
}

// Every visual class in this app is a flat, single-purpose name (same BEM
// convention as mithril-app/src/style.css, which avoids descendant
// selectors). Appending a "--light" sibling to each requested class name
// lets style.css layer light-mode overrides without introducing CSS
// features (custom properties, combinators) not already proven on-device.
function withTheme(classNames: string): string {
  if (state.theme !== "light") return classNames;
  return classNames
    .split(" ")
    .flatMap((name) => [name, `${name}--light`])
    .join(" ");
}

// Not `.toLocaleString("es-CL")`: the Lynx background engine's Intl support
// is unreliable (same class of gap as Array.prototype.at() — see
// mithril-lynx's AGENTS.md), and in practice this rendered a nonsensical
// MM/DD/YYYY-ish date instead of a real es-CL one. Also no need for a full
// date at all: the day-based cache above means `updatedAt` is
// unconditionally re-fetched (loadIndicadores()) the moment it's not from
// today — this timestamp is ALWAYS today's, never "Ayer" or older. Showing
// just the time avoids both problems.
function formatUpdatedAt(timestamp: number | undefined): string {
  if (timestamp == null) return "";
  const d = new Date(timestamp);
  const hh = String(d.getHours()).padStart(2, "0");
  const mm = String(d.getMinutes()).padStart(2, "0");
  const ss = String(d.getSeconds()).padStart(2, "0");
  return `Hoy, ${hh}:${mm}:${ss}`;
}

const MESES = ["Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"];

function formatToday(): string {
  const now = new Date();
  return `Hoy, ${now.getDate()} de ${MESES[now.getMonth()]} ${now.getFullYear()}`;
}

function ThemeToggle() {
  const label = state.theme === "dark" ? "☀️ Modo claro" : "🌙 Modo oscuro";
  return m(
    "view",
    {
      class: withTheme("ThemeToggle"),
      ontap: () => {
        state.theme = state.theme === "dark" ? "light" : "dark";
        redraw();
        persistTheme(state.theme);
      },
    },
    [m("text", { class: withTheme("ThemeToggle-label") }, label)],
  );
}

function ThemedButton(label: string, variant: "primary" | "secondary" = "primary") {
  const buttonClass = variant === "secondary" ? "Button Button--secondary" : "Button";
  const labelClass = variant === "secondary" ? "Button-label Button-label--secondary" : "Button-label";
  return m("view", { class: withTheme(buttonClass), ontap: () => loadIndicadores() }, [
    m("text", { class: withTheme(labelClass) }, label),
  ]);
}

// style.css's .Card carries a fade-in-up entrance animation but can't stagger
// it per card itself — Lynx has no :nth-child() (confirmed against
// lynx-api-docs), so each card gets its own animation-delay inline instead.
// Capped so a long list doesn't make the last card wait forever to appear.
function cardAnimationDelay(index: number): string {
  return `${Math.min(index * 40, 240)}ms`;
}

function IndicadorCard(def: IndicadorDef, value: string | undefined, index: number) {
  return m(
    "view",
    { class: withTheme("Card"), key: def.key, style: { "animation-delay": cardAnimationDelay(index) } },
    [
      m("text", { class: withTheme("Card-label") }, def.label),
      m("text", { class: withTheme("Card-value") }, value ?? "—"),
      m("text", { class: withTheme("Card-unit") }, def.unit),
    ],
  );
}

// Shown only while indicadores is still unset — with the day-based cache
// above, that's just the very first launch of the day, or a first-ever
// install; every other open renders the cached list immediately, no
// network wait, no skeleton.
function SkeletonCard(def: IndicadorDef, index: number) {
  return m(
    "view",
    { class: withTheme("Card"), key: def.key, style: { "animation-delay": cardAnimationDelay(index) } },
    [m("view", { class: withTheme("Skeleton-label") }), m("view", { class: withTheme("Skeleton-value") })],
  );
}

const App: m.Component = {
  view() {
    console.log("[timing] view()", Date.now(), "indicadores=", state.indicadores == null ? "null" : "set");
    const indicadores = state.indicadores;
    const error = state.error;

    return m("view", { class: withTheme("Page") }, [
      m("view", { class: "TopBar" }, [ThemeToggle()]),
      m("view", { class: "Header" }, [
        m("text", { class: withTheme("Title") }, "Indicadores"),
        m("text", { class: withTheme("Subtitle") }, formatToday()),
      ]),
      error != null
        ? m("view", { class: "StateBox" }, [
            m("text", { class: withTheme("StateBox-text StateBox-text--error") }, error),
            ThemedButton("Reintentar"),
          ])
        : indicadores == null
          ? m(
              "view",
              { class: "List" },
              INDICADORES.map((def, i) => SkeletonCard(def, i)),
            )
          : m(
              "view",
              { class: "List" },
              INDICADORES.map((def, i) => IndicadorCard(def, indicadores[def.key], i)),
            ),
      m("view", { class: "Footer" }, [
        m(
          "text",
          { class: withTheme("Footer-updated") },
          state.updatedAt != null ? `Actualizado: ${formatUpdatedAt(state.updatedAt)}` : "",
        ),
        ThemedButton(state.loading ? "Actualizando…" : "Refrescar", "secondary"),
        m("text", { class: withTheme("Footer-source") }, "Fuente: Banco Central de Chile (bcentral.cl)"),
      ]),
    ]);
  },
};

const cached = readCache();
console.log("[timing] cache read", Date.now(), "hit=", cached != null && cached.date === todayKey());
if (cached && cached.date === todayKey()) {
  state.indicadores = cached.indicadores;
  state.updatedAt = cached.updatedAt;
} else {
  loadIndicadores();
}

export default { App, root: () => m(App) };
