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
    // build.zig.zon (C puro + port posix).
    // ------------------------------------------------------------------
    const lexbor_dep = b.dependency("lexbor", .{});

    const lexbor_lib = b.addLibrary(.{
        .name = "lexbor",
        .linkage = .static,
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = true,
        }),
    });
    lexbor_lib.root_module.addIncludePath(lexbor_dep.path("source"));

    const lexbor_sources = collectLexborSources(b, lexbor_dep);
    lexbor_lib.root_module.addCSourceFiles(.{
        .root = lexbor_dep.path("source"),
        .files = lexbor_sources,
        .flags = &.{"-std=c99"},
    });

    // ------------------------------------------------------------------
    // Modulo "c": translate-c (reemplazo de @cImport, deprecado en 0.16).
    // ------------------------------------------------------------------
    const translate_c = b.addTranslateC(.{
        .root_source_file = b.path("src/c.h"),
        .target = target,
        .optimize = optimize,
    });
    translate_c.addIncludePath(lexbor_dep.path("source"));

    // ------------------------------------------------------------------
    // Ejecutable
    // ------------------------------------------------------------------
    const exe_mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
        .imports = &.{
            .{ .name = "c", .module = translate_c.createModule() },
        },
    });
    exe_mod.addOptions("build_options", build_options);
    exe_mod.linkLibrary(lexbor_lib);

    const exe = b.addExecutable(.{
        .name = "indicadores",
        .root_module = exe_mod,
    });
    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| run_cmd.addArgs(args);
    const run_step = b.step("run", "Ejecuta el scraper");
    run_step.dependOn(&run_cmd.step);

    // ------------------------------------------------------------------
    // Tests (logica pura: cleanValue, toLowercase, etc.)
    // ------------------------------------------------------------------
    const test_mod = b.createModule(.{
        .root_source_file = b.path("src/scrapy.zig"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
        .imports = &.{
            .{ .name = "c", .module = translate_c.createModule() },
        },
    });
    test_mod.addOptions("build_options", build_options);
    const unit_tests = b.addTest(.{ .root_module = test_mod });
    unit_tests.root_module.linkLibrary(lexbor_lib);

    const run_tests = b.addRunArtifact(unit_tests);
    const test_step = b.step("test", "Corre los tests unitarios");
    test_step.dependOn(&run_tests.step);
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
