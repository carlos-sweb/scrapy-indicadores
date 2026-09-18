const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // Directorio "home" del binario, usado para la caché (paridad con
    // INSTALL_BIN_DIR del CMakeLists original).
    const install_bin_dir = b.option(
        []const u8,
        "install-bin-dir",
        "Directorio base para la cache (.scrapy-indicadores)",
    ) orelse "/opt/indicadores/bin";

    const build_options = b.addOptions();
    build_options.addOption([]const u8, "install_bin_dir", install_bin_dir);

    // ------------------------------------------------------------------
    // Build normal: target/optimize elegidos por linea de comandos
    // (nativo por defecto).
    // ------------------------------------------------------------------
    const main_build = buildIndicadores(b, .{
        .target = target,
        .optimize = optimize,
        .build_options = build_options,
    });
    b.installArtifact(main_build.exe);

    const run_cmd = b.addRunArtifact(main_build.exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| run_cmd.addArgs(args);
    const run_step = b.step("run", "Ejecuta el scraper");
    run_step.dependOn(&run_cmd.step);

    // ------------------------------------------------------------------
    // Tests (logica pura: cleanValue, toLowercase, etc.).
    // ------------------------------------------------------------------
    const z_lexbor_dep = b.dependency("z_lexbor", .{ .target = target, .optimize = optimize });

    const test_mod = b.createModule(.{
        .root_source_file = b.path("src/scrapy.zig"),
        .target = target,
        .optimize = optimize,
        .imports = &.{
            .{ .name = "z_lexbor", .module = z_lexbor_dep.module("z_lexbor") },
        },
    });
    test_mod.addOptions("build_options", build_options);
    const unit_tests = b.addTest(.{ .root_module = test_mod });

    const run_tests = b.addRunArtifact(unit_tests);
    const test_step = b.step("test", "Corre los tests unitarios");
    test_step.dependOn(&run_tests.step);

    // ------------------------------------------------------------------
    // `zig build exe-musl`: cross-compila x86_64-linux-musl ReleaseSmall
    // y copia el binario directo al arbol fuente en bin/indicadores-x86_64
    // -- el mismo par de comandos que documenta el README ("Publicar el
    // binario del CI"), ahora como paso de build en vez de manual.
    // `addUpdateSourceFiles`/`addCopyFileToSource` escriben en el arbol
    // fuente a proposito (no en zig-out); sigue faltando el
    // `git add`/commit, que se deja manual e intencional.
    // ------------------------------------------------------------------
    const musl_target = b.resolveTargetQuery(.{
        .cpu_arch = .x86_64,
        .os_tag = .linux,
        .abi = .musl,
    });
    const musl_build = buildIndicadores(b, .{
        .target = musl_target,
        .optimize = .ReleaseSmall,
        .build_options = build_options,
    });

    const update_musl_bin = b.addUpdateSourceFiles();
    update_musl_bin.addCopyFileToSource(musl_build.exe.getEmittedBin(), "bin/indicadores-x86_64");
    const musl_step = b.step("exe-musl", "Cross-compila x86_64-linux-musl ReleaseSmall -> bin/indicadores-x86_64 (falta git add + commit)");
    musl_step.dependOn(&update_musl_bin.step);
}

const IndicadoresBuild = struct {
    exe: *std.Build.Step.Compile,
};

const BuildOpts = struct {
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    build_options: *std.Build.Step.Options,
};

/// Arma el ejecutable "indicadores" para un target/optimize dados.
/// Factorizado para poder repetirlo con un target distinto (`exe-musl`)
/// sin duplicar la definicion del build.
fn buildIndicadores(b: *std.Build, opts: BuildOpts) IndicadoresBuild {
    const z_lexbor_dep = b.dependency("z_lexbor", .{ .target = opts.target, .optimize = opts.optimize });
    const zargs_dep = b.dependency("zargs", .{ .target = opts.target, .optimize = opts.optimize });

    const exe_mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = opts.target,
        .optimize = opts.optimize,
        .link_libc = true,
        .imports = &.{
            .{ .name = "z_lexbor", .module = z_lexbor_dep.module("z_lexbor") },
            .{ .name = "zargs", .module = zargs_dep.module("zargs") },
        },
    });
    exe_mod.addOptions("build_options", opts.build_options);

    const exe = b.addExecutable(.{
        .name = "indicadores",
        .root_module = exe_mod,
    });

    return .{ .exe = exe };
}
