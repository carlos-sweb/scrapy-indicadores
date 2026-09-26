#!/usr/bin/env node
/**
 * Bridge between this JS project (the Lynx bundle) and its sibling Android host
 * (`indicadores-android/`).
 *
 * Automates Part C of ANDROID_APK_GUIDE.md: build the bundle, copy it (and
 * dist/static/) into `app/src/main/assets/`, then build/install and launch
 * the APK. Adapted from create-mithril-lynx's template (android.mjs); the
 * only change is syncing dist/static/ instead of src/assets/fonts/.
 *
 * Release signing reads indicadores-android/keystore.properties (gitignored),
 * which points at the existing release keystore — never regenerate it: an
 * update must be signed with the same key as the installed app.
 *
 *   npm run android            build + sync + installDebug + launch on the device
 *   npm run android:apk        build + sync + assembleDebug (build the APK only)
 *   npm run android:release    build + sync + assembleRelease (signs if keystore.properties exists)
 *   npm run android:sync       build + sync into assets only (no Gradle)
 *
 * Extra flags, passed after `--`:
 *   npm run android -- --no-build        reuse dist/ as-is (don't rebuild)
 *   npm run android -- --no-launch       don't launch the app over adb at the end
 *   npm run android:apk -- -- --info     everything after `--` is forwarded to ./gradlew
 */
import { spawnSync } from "node:child_process";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const projectRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");

/** Sibling Android host, resolved when the project was generated. */
const ANDROID_DIR = path.resolve(projectRoot, "../indicadores-android");

const APPLICATION_ID = "com.carlossweb.indicadores";
const ACTIVITY_CLASS = ".MainActivity";
const BUNDLE_NAME = "main-thread.bundle";

const BUNDLE_SRC = path.join(projectRoot, "dist", BUNDLE_NAME);
const BUNDLE_DEST = path.join(ANDROID_DIR, "app", "src", "main", "assets", BUNDLE_NAME);

// Static assets emitted by rspeedy next to the bundle (the Ubuntu Sans Mono
// font, as dist/static/font/<name>.<hash>.ttf). This app imports the font
// and lets rspeedy's `output.assetPrefix: "asset:///"` (lynx.config.ts) turn
// it into asset:///static/font/... — resolved in the APK by the host's
// AssetFontFaceLoader. So the host's assets/static/ must mirror dist/static/
// exactly (a stale hashed file left behind would just bloat the APK).
const STATIC_SRC = path.join(projectRoot, "dist", "static");
const STATIC_DEST = path.join(ANDROID_DIR, "app", "src", "main", "assets", "static");

const USAGE = `
Usage: npm run android -- [flags] [-- gradle-args]

  --apk          build the debug APK (assembleDebug) instead of installing it
  --release      build the release APK (assembleRelease, signed if keystore.properties exists)
  --sync-only    only build the bundle and copy it into assets/ (no Gradle)
  --no-build     don't rebuild the bundle; use dist/ as it is
  --no-launch    don't launch the app over adb at the end
  --keystore     (disabled: this app reuses its existing release key)
  --help         print this
`.trim();

const args = process.argv.slice(2);
const flags = new Set(args.filter((a) => a.startsWith("-")));

function afterDoubleDash() {
	const i = args.indexOf("--");
	return i === -1 ? [] : args.slice(i + 1);
}

function fail(message) {
	console.error(`\n  ✖ ${message}\n`);
	process.exit(1);
}

function run(command, commandArgs, options = {}) {
	const result = spawnSync(command, commandArgs, {
		cwd: options.cwd ?? projectRoot,
		stdio: options.capture ? "pipe" : "inherit",
		shell: process.platform === "win32",
		encoding: "utf8",
	});
	if (result.error) {
		fail(`Could not run "${command}": ${result.error.message}`);
	}
	if (result.status !== 0) {
		fail(`"${command} ${commandArgs.join(" ")}" exited with code ${result.status}.`);
	}
	return result.stdout ?? "";
}

function detectPackageManager() {
	const userAgent = process.env.npm_config_user_agent ?? "";
	if (userAgent.startsWith("bun")) return "bun";
	if (userAgent.startsWith("pnpm")) return "pnpm";
	if (userAgent.startsWith("yarn")) return "yarn";
	return "npm";
}

function gradlew() {
	return process.platform === "win32" ? "gradlew.bat" : "./gradlew";
}

function requireAndroidDir() {
	if (!fs.existsSync(path.join(ANDROID_DIR, "settings.gradle.kts"))) {
		fail(
			`Can't find the Android host at ${ANDROID_DIR}.\n` +
				`    Generate it with:  npm create mithril-lynx@latest <name> --android\n` +
				`    (or pass --android-id/--app-name so it doesn't prompt).`,
		);
	}
}

function listFiles(dir) {
	if (!fs.existsSync(dir)) return [];
	return fs.readdirSync(dir, { recursive: true }).filter((rel) => fs.statSync(path.join(dir, rel)).isFile());
}

function syncStatic() {
	const wanted = new Set(listFiles(STATIC_SRC));
	for (const rel of listFiles(STATIC_DEST)) {
		if (!wanted.has(rel)) {
			fs.rmSync(path.join(STATIC_DEST, rel));
			console.log(`  ✗ removed stale ${path.relative(projectRoot, path.join(STATIC_DEST, rel))}`);
		}
	}
	for (const rel of wanted) {
		const dest = path.join(STATIC_DEST, rel);
		fs.mkdirSync(path.dirname(dest), { recursive: true });
		fs.copyFileSync(path.join(STATIC_SRC, rel), dest);
		const kb = (fs.statSync(dest).size / 1024).toFixed(1);
		console.log(`  → ${path.relative(projectRoot, dest)} (${kb} kB)`);
	}
}

function syncBundle() {
	if (!fs.existsSync(BUNDLE_SRC)) {
		fail(`${path.relative(projectRoot, BUNDLE_SRC)} doesn't exist. Run "npm run build" first (or drop --no-build).`);
	}
	requireAndroidDir();
	fs.mkdirSync(path.dirname(BUNDLE_DEST), { recursive: true });
	fs.copyFileSync(BUNDLE_SRC, BUNDLE_DEST);
	const kb = (fs.statSync(BUNDLE_DEST).size / 1024).toFixed(1);
	console.log(`  → ${path.relative(projectRoot, BUNDLE_DEST)} (${kb} kB)`);
	syncStatic();
}

function generateKeystore() {
	// Deliberately not generating anything here, unlike the template: this app
	// is already published, and Android refuses an update signed with a
	// different key. keystore.properties must point at the existing release
	// keystore (kept outside the repo) — see the header comment.
	fail(
		"This app already has a release key. Create indicadores-android/keystore.properties " +
			"(storeFile, storePassword, keyAlias, keyPassword) pointing at the existing keystore " +
			"instead of generating a new one.",
	);
}

function buildBundle() {
	const manager = detectPackageManager();
	console.log(`\n▸ Building the bundle (${manager} run build)…`);
	run(manager, ["run", "build"]);
}

function runGradle(task, extraArgs) {
	console.log(`\n▸ ./gradlew ${task}…`);
	run(gradlew(), [task, ...extraArgs], { cwd: ANDROID_DIR });
}

function launch() {
	console.log("\n▸ Launching on the device…\n");
	run("adb", ["shell", "am", "force-stop", APPLICATION_ID]);
	const output = run("adb", ["shell", "am", "start", "-W", "-n", `${APPLICATION_ID}/${ACTIVITY_CLASS}`], {
		capture: true,
	});
	for (const line of output.split("\n")) {
		if (/TotalTime|WaitTime|LaunchState|Error|Exception/.test(line)) console.log(`  ${line.trim()}`);
	}
	console.log("\n  Logs:  adb logcat | grep -i lynx");
}

function main() {
	if (flags.has("--help") || flags.has("-h")) {
		console.log(USAGE);
		return;
	}
	if (flags.has("--keystore")) {
		generateKeystore();
		return;
	}

	const syncOnly = flags.has("--sync-only");

	if (!flags.has("--no-build")) buildBundle();
	syncBundle();

	if (syncOnly) {
		console.log("\n  ✔ Assets synced.\n");
		return;
	}

	const extraGradleArgs = afterDoubleDash();
	if (flags.has("--release")) {
		runGradle("assembleRelease", extraGradleArgs);
		const apkDir = path.join(ANDROID_DIR, "app", "build", "outputs", "apk", "release");
		for (const apk of fs.existsSync(apkDir) ? fs.readdirSync(apkDir).filter((f) => f.endsWith(".apk")) : []) {
			console.log(`\n  ✔ APK: ${path.relative(projectRoot, path.join(apkDir, apk))}`);
		}
	} else if (flags.has("--apk") || process.env.CI) {
		runGradle("assembleDebug", extraGradleArgs);
		console.log(
			`\n  ✔ APK: ${path.relative(projectRoot, path.join(ANDROID_DIR, "app", "build", "outputs", "apk", "debug", "app-debug.apk"))}`,
		);
	} else {
		runGradle("installDebug", extraGradleArgs);
	}

	if (!flags.has("--no-launch") && !flags.has("--apk") && !flags.has("--release")) {
		launch();
	} else {
		console.log("");
	}
}

main();
