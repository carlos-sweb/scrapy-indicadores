// background.ts
//
// Entry point for the background (JS) thread — the only thread that ever
// runs a component's view() or Mithril's diff (mithril-lynx/src/background.js's
// own header). index.ts owns the entire app: state, the Banco Central fetch,
// and the view. This file just boots it.
import { renderApp } from "mithril-lynx/background";
import app from "./index.js";

renderApp({ root: app.root });
