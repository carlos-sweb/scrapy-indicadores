import m from "mithril";
import shim from "mithril-lynx";
import { getData, dispatchToBackground } from "mithril-lynx/main-thread";

interface Indicadores {
  UF?: string;
  Dolar?: string;
  Euro?: string;
  Yen?: string;
  Oro?: string;
  Plata?: string;
  Cobre?: string;
}

interface AppData {
  indicadores?: Indicadores;
  updatedAt?: number;
  loading?: boolean;
  error?: string;
  theme?: "dark" | "light";
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

// Main-thread-owned UI state — mithril-lynx never auto-redraws after an
// on* event (see lynx-mithril-shim.js: every normalized event carries
// `redraw: false` on purpose, to avoid a redraw on every touch event) — so
// a local mutation still needs its own explicit shim.redraw(), same as
// mithril-lynx/navigation's push/pop/replace do internally.
//
// The *initial* value comes from initData (see MainActivity.kt's
// readInitData()): the native host reads the last saved theme from
// SharedPreferences synchronously, before renderTemplateUrl(), so the very
// first frame already paints in the right theme — no flash of dark before
// switching to light. seedThemeFromData() below applies that seed exactly
// once, on the first view(); every toggle after that is local state plus a
// fire-and-forget dispatchToBackground("themeChanged", theme) so
// background.ts can persist the new choice for the *next* cold start.
type Theme = "dark" | "light";
let theme: Theme = "dark";
let themeSeeded = false;

function seedThemeFromData(data: AppData | undefined) {
  if (themeSeeded) return;
  themeSeeded = true;
  if (data?.theme === "dark" || data?.theme === "light") theme = data.theme;
}

// Every visual class in this app is a flat, single-purpose name (same BEM
// convention as mithril-app/src/style.css, which avoids descendant
// selectors). Appending a "--light" sibling to each requested class name
// lets style.css layer light-mode overrides without introducing CSS
// features (custom properties, combinators) not already proven on-device.
function withTheme(classNames: string): string {
  if (theme !== "light") return classNames;
  return classNames
    .split(" ")
    .flatMap((name) => [name, `${name}--light`])
    .join(" ");
}

function formatUpdatedAt(timestamp: number | undefined): string {
  if (timestamp == null) return "";
  try {
    return new Date(timestamp).toLocaleString("es-CL");
  } catch {
    return new Date(timestamp).toISOString();
  }
}

const MESES = [
  "Ene",
  "Feb",
  "Mar",
  "Abr",
  "May",
  "Jun",
  "Jul",
  "Ago",
  "Sep",
  "Oct",
  "Nov",
  "Dic",
];

function formatToday(): string {
  const now = new Date();
  return `Hoy, ${now.getDate()} de ${MESES[now.getMonth()]} ${now.getFullYear()}`;
}

function ThemeToggle() {
  const label = theme === "dark" ? "☀️ Modo claro" : "🌙 Modo oscuro";
  return m(
    "view",
    {
      class: withTheme("ThemeToggle"),
      ontap: () => {
        theme = theme === "dark" ? "light" : "dark";
        shim.redraw();
        dispatchToBackground("themeChanged", theme);
      },
    },
    [m("text", { class: withTheme("ThemeToggle-label") }, label)],
  );
}

function ThemedButton(label: string, variant: "primary" | "secondary" = "primary") {
  const buttonClass = variant === "secondary" ? "Button Button--secondary" : "Button";
  const labelClass = variant === "secondary" ? "Button-label Button-label--secondary" : "Button-label";
  return m(
    "view",
    { class: withTheme(buttonClass), ontap: () => dispatchToBackground("refresh") },
    [m("text", { class: withTheme(labelClass) }, label)],
  );
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

// Shown only while indicadores.length===0 with no cached data yet — with
// background.ts's day-based cache (see its own header comment), that's just
// the very first launch of the day, or a first-ever install; every other
// open renders the cached list immediately, no network wait, no skeleton.
function SkeletonCard(def: IndicadorDef, index: number) {
  return m(
    "view",
    { class: withTheme("Card"), key: def.key, style: { "animation-delay": cardAnimationDelay(index) } },
    [m("view", { class: withTheme("Skeleton-label") }), m("view", { class: withTheme("Skeleton-value") })],
  );
}

const App: m.Component = {
  view() {
    const data = getData<AppData>();
    seedThemeFromData(data);
    const indicadores = data?.indicadores;
    const error = data?.error;

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
          data?.updatedAt != null ? `Actualizado: ${formatUpdatedAt(data.updatedAt)}` : "",
        ),
        ThemedButton(data?.loading ? "Actualizando…" : "Refrescar", "secondary"),
        m(
          "text",
          { class: withTheme("Footer-source") },
          "Fuente: Banco Central de Chile (bcentral.cl)",
        ),
      ]),
    ]);
  },
};

export default { App, root: () => m(App) };
