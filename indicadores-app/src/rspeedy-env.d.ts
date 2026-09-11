/// <reference types="@lynx-js/rspeedy/client" />
/// <reference types="@lynx-js/types" />
/// <reference types="@lynx-js/type-element-api" />

// indicadores-android's IndicadoresStorageModule.kt — a persistent
// key-value store registered via LynxViewBuilder.registerModule() on the
// native side (see MainActivity.kt). @lynx-js/types' own NativeModules
// interface already has a `[key: string]: any` index signature, so this
// isn't strictly required for background.ts to compile, but it gives real
// autocomplete/type-checking on the two methods that actually exist.
declare module "@lynx-js/types" {
  interface NativeModules {
    IndicadoresStorageModule?: {
      get(key: string): string | null;
      set(key: string, value: string): void;
    };
  }
}
