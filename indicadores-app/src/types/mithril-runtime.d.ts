// "mithril-runtime" (https://github.com/carlos-sweb/mithril-runtime) ships
// no types of its own yet — same shim demo-ui and mithril-lynx-ui use
// internally, needed here because this app imports "mithril-runtime"
// directly.
declare module "mithril-runtime" {
  import m from "mithril";
  export = m;
}
