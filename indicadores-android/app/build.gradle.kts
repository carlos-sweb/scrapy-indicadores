import java.util.Properties

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

// Loaded from a gitignored keystore.properties (see indicadores-android/README or
// the project's own memory notes for how to generate one) — never hardcode
// signing secrets here. Its absence (e.g. a fresh clone) must not break debug
// builds, so every read below is guarded by releaseSigningPropsFile.exists().
val releaseSigningPropsFile = rootProject.file("keystore.properties")
val releaseSigningProps = Properties().apply {
    if (releaseSigningPropsFile.exists()) {
        releaseSigningPropsFile.inputStream().use { load(it) }
    }
}

android {
    namespace = "com.carlossweb.indicadores"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.carlossweb.indicadores"
        minSdk = 24
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"
    }

    signingConfigs {
        if (releaseSigningPropsFile.exists()) {
            create("release") {
                storeFile = file(releaseSigningProps.getProperty("storeFile"))
                storePassword = releaseSigningProps.getProperty("storePassword")
                keyAlias = releaseSigningProps.getProperty("keyAlias")
                keyPassword = releaseSigningProps.getProperty("keyPassword")
            }
        }
    }

    buildTypes {
        debug {
            isMinifyEnabled = false
        }
        release {
            // Kept off for this first release — matches the debug config, and
            // avoids risking a first-ever R8/minify pass against Lynx's JNI
            // bridge on the very release being published. Revisit later.
            isMinifyEnabled = false
            if (releaseSigningPropsFile.exists()) {
                signingConfig = signingConfigs.getByName("release")
            }
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    kotlinOptions {
        jvmTarget = "17"
    }
}

dependencies {
    // The lynx artifact's own POM pulls in lynx-base, lynx-gfx, lynx-trace,
    // primjs, primjsWasm, lynx-jssdk, service-api transitively — see
    // https://repo1.maven.org/maven2/org/lynxsdk/lynx/lynx/4.1.0/lynx-4.1.0.pom
    implementation("org.lynxsdk.lynx:lynx:4.1.0")
    // lynxtextra removed 2026-09-10 — the isolated indicadores-react-android
    // A/B test (same font, same fetch, no lynxtextra/devtool) cold-started
    // in ~750-800ms vs. this app's ~2.2-2.4s, so lynxtextra was never the
    // explanation; keeping it on the classpath was dead weight.
    // Without this, lynx.fetch() (indicadores-app's background.ts, fetching
    // scrapy-indicadores' data.json) fails at runtime with "Lynx Http
    // Service Not registered" — confirmed on real hardware 2026-09-10. The
    // core `lynx` artifact only defines the service *interface*; a concrete
    // implementation is a separate, opt-in artifact, registered in
    // IndicadoresApp.onCreate().
    implementation("org.lynxsdk.lynx:lynx-service-http:4.1.0")
    // lynx-service-http's LynxHttpService.kt uses OkHttp directly but its
    // own POM declares no okhttp dependency at all (the consumer is
    // expected to bring their own version) — without this, the app crashes
    // on startup with NoClassDefFoundError: Lokhttp3/OkHttpClient, confirmed
    // on real hardware 2026-09-10.
    implementation("com.squareup.okhttp3:okhttp:4.12.0")
    implementation("androidx.appcompat:appcompat:1.7.0")
    // Official Android SplashScreen API (backported to minSdk 24 by this
    // library) — shows the branded splash automatically from process start
    // until the first frame is drawn, no manual delay/timer needed. See
    // MainActivity.kt's installSplashScreen() call and themes.xml.
    implementation("androidx.core:core-splashscreen:1.0.1")
    // lynx-devtool/lynx-service-devtool removed 2026-09-10 — added for one
    // investigation (fixed a real JNI crash, see IndicadoresApp.kt's git
    // history) but never actually got agent-lynx's DebugRouter client
    // discovery working, and the isolated A/B test without them was already
    // ~3x faster than this app with them — not the bottleneck, just extra
    // .so/dex weight.
    // lynx-service-log, xelement/xelement-input (navigation + calculator
    // feature, <input> support): added and then reverted 2026-09-10 along
    // with that feature.
}
