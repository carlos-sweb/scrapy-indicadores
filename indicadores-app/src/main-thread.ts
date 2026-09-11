import { setupApp } from "mithril-lynx/main-thread";
import app from "./index.js";

// Data-channel mode (see mithril-lynx/README.md, "Usage — data-channel
// mode"): background.ts owns the network call (lynx.fetch only exists on
// the background thread — see README's "Known gap, not permanent" note on
// m.request, which documents this same constraint for Mithril's own API),
// this file just mounts the Mithril root and lets shim.redraw() pick up
// every later push from background.ts's setData().
setupApp({ root: app.root });
