import path from "node:path";
import { fileURLToPath } from "node:url";
import { pluginLynxConfig } from '@lynx-js/config-rsbuild-plugin';
import { pluginQRCode } from "@lynx-js/qrcode-rsbuild-plugin";
import { defineConfig } from "@lynx-js/rspeedy";
import { pluginTypeCheck } from "@rsbuild/plugin-type-check";

import { pluginMithrilLynx } from "mithril-lynx/plugin";

const projectRoot = path.dirname(fileURLToPath(import.meta.url));

export default defineConfig({
  source: {
    entry: {
      // mithril-lynx/plugin's own modifyBundlerChain finds this entry's
      // sibling background.ts automatically and builds both bundles from
      // this single declaration — see src/background.ts's own header.
      "main-thread": path.join(projectRoot, "src/main-thread.ts"),
    },
  },
  output: {
    distPath: {
      root: path.join(projectRoot, "dist"),
    },
    filename: "[name].bundle",
    // Only takes effect in production (dev.assetPrefix covers dev — see
    // https://lynxjs.org/api/rspeedy/rspeedy.dev.assetprefix.html). Without
    // this, imported static assets (the .ttf font) resolve to a URL that
    // means nothing inside the packaged Android app (no HTTP server there).
    // "asset:///" matches the same scheme indicadores-android's own asset
    // resolution already understands — see
    // https://lynxjs.org/rspeedy/assets.
    assetPrefix: "asset:///",
  },
  plugins: [
    pluginMithrilLynx(),
    pluginLynxConfig({ enableCSSRule: true }),
    // Deliberately no pluginLynxConfig({ enableCSSRule: true }): confirmed
    // on device, isolated against both mithril-lynx v1 and v2 and both
    // template-webpack-plugin 0.16.0/0.16.1, that this flag — not the
    // framework version, not the encoder version — is what makes the
    // native engine hang indefinitely (tight "unknown Decode Error"/
    // ReadStringDirectly loop in binary_reader.cc, then an ANR) whenever
    // style.css's @font-face (asset:///fonts/ubuntu_mono.ttf, only
    // resolvable inside the real Android host) is compiled in and opened
    // on any other host, including LynxExplorer/Lynx Go opening the QR
    // code published on the site. Cost of leaving it off: style.css's
    // @keyframes (fade-in, fade-in-up, skeleton-pulse) don't compile in —
    // a cosmetic loss, accepted in exchange for one single build that
    // works everywhere instead of two build targets + an aliased-away
    // font file.
    pluginQRCode({
      schema(url) {
        // Opens the page in LynxExplorer in full screen mode.
        return `${url}?fullscreen=true`;
      },
    }),
    pluginTypeCheck(),
  ],
});
