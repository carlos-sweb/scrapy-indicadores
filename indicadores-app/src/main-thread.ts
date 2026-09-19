// main-thread.ts
//
// Entry point for the Lepus VM. setupRenderer() only interprets patches from
// the background thread against real Element PAPI — it never runs Mithril
// or this app's view() (see background.ts's own header). Nothing app-
// specific belongs here: every piece of state, the fetch to Banco Central,
// and the whole view live in index.ts instead, imported by background.ts.
import { setupRenderer } from "mithril-lynx/main-thread";

setupRenderer();
