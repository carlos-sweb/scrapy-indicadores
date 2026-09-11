import path from "node:path";
import { fileURLToPath } from "node:url";

import { pluginQRCode } from "@lynx-js/qrcode-rsbuild-plugin";
import { defineConfig } from "@lynx-js/rspeedy";
import { pluginTypeCheck } from "@rsbuild/plugin-type-check";

import { pluginMithrilLynx } from "mithril-lynx/plugin";

const projectRoot = path.dirname(fileURLToPath(import.meta.url));

export default defineConfig({
  source: {
    entry: {
      "main-thread": path.join(projectRoot, "src/main-thread.ts"),
    },
  },
  output: {
    distPath: {
      root: path.join(projectRoot, "dist"),
    },
    filename: "[name].bundle",
  },
  plugins: [
    pluginMithrilLynx(),
    pluginQRCode({
      schema(url) {
        // Opens the page in LynxExplorer in full screen mode.
        return `${url}?fullscreen=true`;
      },
    }),
    pluginTypeCheck(),
  ],
});
