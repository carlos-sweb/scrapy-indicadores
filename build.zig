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
    // lexbor: lib estatica compilada desde el tarball declarado en
    // build.zig.zon (C puro + port posix). La lista de fuentes es la
    // misma sin importar el target, asi que se recolecta una sola vez.
    // ------------------------------------------------------------------
    const lexbor_dep = b.dependency("lexbor", .{});
    const lexbor_sources = collectLexborSources(b, lexbor_dep);

    // ------------------------------------------------------------------
    // Build normal: target/optimize elegidos por linea de comandos
    // (nativo por defecto).
    // ------------------------------------------------------------------
    const main_build = buildIndicadores(b, .{
        .target = target,
        .optimize = optimize,
        .lexbor_dep = lexbor_dep,
        .lexbor_sources = lexbor_sources,
        .build_options = build_options,
    });
    b.installArtifact(main_build.exe);

    const run_cmd = b.addRunArtifact(main_build.exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| run_cmd.addArgs(args);
    const run_step = b.step("run", "Ejecuta el scraper");
    run_step.dependOn(&run_cmd.step);

    // ------------------------------------------------------------------
    // Tests (logica pura: cleanValue, toLowercase, etc.) -- reusa el
    // lexbor/translate-c del build normal.
    // ------------------------------------------------------------------
    const test_mod = b.createModule(.{
        .root_source_file = b.path("src/scrapy.zig"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
        .imports = &.{
            .{ .name = "c", .module = main_build.translate_c.createModule() },
        },
    });
    test_mod.addOptions("build_options", build_options);
    const unit_tests = b.addTest(.{ .root_module = test_mod });
    unit_tests.root_module.linkLibrary(main_build.lexbor_lib);

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
        .lexbor_dep = lexbor_dep,
        .lexbor_sources = lexbor_sources,
        .build_options = build_options,
    });

    const update_musl_bin = b.addUpdateSourceFiles();
    update_musl_bin.addCopyFileToSource(musl_build.exe.getEmittedBin(), "bin/indicadores-x86_64");
    const musl_step = b.step("exe-musl", "Cross-compila x86_64-linux-musl ReleaseSmall -> bin/indicadores-x86_64 (falta git add + commit)");
    musl_step.dependOn(&update_musl_bin.step);
}

const IndicadoresBuild = struct {
    exe: *std.Build.Step.Compile,
    lexbor_lib: *std.Build.Step.Compile,
    translate_c: *std.Build.Step.TranslateC,
};

const BuildOpts = struct {
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    lexbor_dep: *std.Build.Dependency,
    lexbor_sources: []const []const u8,
    build_options: *std.Build.Step.Options,
};

/// Arma lexbor + el modulo "c" (translate-c) + el ejecutable "indicadores"
/// para un target/optimize dados. Factorizado para poder repetirlo con un
/// target distinto (`exe-musl`) sin duplicar la definicion del build.
fn buildIndicadores(b: *std.Build, opts: BuildOpts) IndicadoresBuild {
    const lexbor_lib = b.addLibrary(.{
        .name = "lexbor",
        .linkage = .static,
        .root_module = b.createModule(.{
            .target = opts.target,
            .optimize = opts.optimize,
            .link_libc = true,
        }),
    });
    lexbor_lib.root_module.addIncludePath(opts.lexbor_dep.path("source"));
    lexbor_lib.root_module.addCSourceFiles(.{
        .root = opts.lexbor_dep.path("source"),
        .files = opts.lexbor_sources,
        .flags = &.{"-std=c99"},
    });

    const translate_c = b.addTranslateC(.{
        .root_source_file = b.path("src/c.h"),
        .target = opts.target,
        .optimize = opts.optimize,
    });
    translate_c.addIncludePath(opts.lexbor_dep.path("source"));

    const zargs_dep = b.dependency("zargs", .{ .target = opts.target, .optimize = opts.optimize });

    const exe_mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = opts.target,
        .optimize = opts.optimize,
        .link_libc = true,
        .imports = &.{
            .{ .name = "c", .module = translate_c.createModule() },
            .{ .name = "zargs", .module = zargs_dep.module("zargs") },
        },
    });
    exe_mod.addOptions("build_options", opts.build_options);
    exe_mod.linkLibrary(lexbor_lib);

    const exe = b.addExecutable(.{
        .name = "indicadores",
        .root_module = exe_mod,
    });

    return .{ .exe = exe, .lexbor_lib = lexbor_lib, .translate_c = translate_c };
}

/// Enumera los .c de lexbor en tiempo de configuracion: todos los modulos
/// mas el port posix (se excluye ports/, que tiene su propia seleccion).
fn collectLexborSources(b: *std.Build, dep: *std.Build.Dependency) []const []const u8 {
    const io = b.graph.io;
    var files: std.ArrayList([]const u8) = .empty;

    const source_root = dep.path("source").getPath2(b, null);
    var dir = std.Io.Dir.openDirAbsolute(io, source_root, .{ .iterate = true }) catch |err| {
        std.debug.panic("no se pudo abrir {s}: {t}", .{ source_root, err });
    };
    defer dir.close(io);

    var walker = dir.walk(b.allocator) catch @panic("OOM");
    defer walker.deinit();

    while (walker.next(io) catch @panic("walk lexbor")) |entry| {
        if (entry.kind != .file) continue;
        if (!std.mem.endsWith(u8, entry.path, ".c")) continue;
        if (std.mem.indexOf(u8, entry.path, "ports/") != null) continue;
        files.append(b.allocator, b.dupe(entry.path)) catch @panic("OOM");
    }

    // Port posix (fs, memory, perf, ...)
    const posix_port = "lexbor/ports/posix";
    var port_dir = dir.openDir(io, posix_port, .{ .iterate = true }) catch |err| {
        std.debug.panic("no se pudo abrir el port posix: {t}", .{err});
    };
    defer port_dir.close(io);

    var port_walker = port_dir.walk(b.allocator) catch @panic("OOM");
    defer port_walker.deinit();

    while (port_walker.next(io) catch @panic("walk posix port")) |entry| {
        if (entry.kind != .file) continue;
        if (!std.mem.endsWith(u8, entry.path, ".c")) continue;
        files.append(b.allocator, b.fmt("{s}/{s}", .{ posix_port, entry.path })) catch @panic("OOM");
    }

    return files.toOwnedSlice(b.allocator) catch @panic("OOM");
}
