//! Indicadores Chile: CLI para obtener indicadores economicos
//! del Banco Central de Chile.
//!
//! Uso:
//!   -h, --help              Muestra la ayuda
//!   -v, --version           Muestra la version
//!   -f, --format <FORMATO>  Formato de salida (table, json, txt, none)
//!   -nc, --no-cache         Sistema de cache
//!   -s, --send <URL>        Envia los datos via POST a la URL
//!   -o, --output <PATH>     Guarda la salida en un archivo
//!   --silent                Modo silencioso (sin salida por consola)
const std = @import("std");
const scrapy = @import("scrapy.zig");
const zargs = @import("zargs");
const Declarative = zargs.Declarative;

const Cli = struct {
    help: Declarative.Flag(bool, .{ .short = 'h', .long = "help", .help = "Muestra la ayuda", .default = false }),
    version: Declarative.Flag(bool, .{ .short = 'v', .long = "version", .help = "Muestra la version", .default = false }),
    format: Declarative.Flag([]const u8, .{ .short = 'f', .long = "format", .help = "Formato de salida (table, json, txt, none)", .default = "table" }),
    cache: Declarative.Flag(bool, .{ .long = "cache", .help = "Sistema de cache (--no-cache para desactivar)", .default = true }),
    send: Declarative.Flag([]const u8, .{ .short = 's', .long = "send", .help = "Envia los datos via POST a la URL", .default = "" }),
    output: Declarative.Flag([]const u8, .{ .short = 'o', .long = "output", .help = "Guarda la salida en un archivo", .default = "" }),
    silent: Declarative.Flag(bool, .{ .long = "silent", .help = "Modo silencioso (sin salida por consola)", .default = false }),
};

/// `-nc` era el short original de `--no-cache`; z-args Declarative solo
/// admite shorts de un caracter, asi que se reescribe antes de parsear.
fn remapLegacyNc(alloc: std.mem.Allocator, args: []const [:0]const u8) ![]const [:0]const u8 {
    const out = try alloc.alloc([:0]const u8, args.len);
    for (args, 0..) |a, idx| out[idx] = if (std.mem.eql(u8, a, "-nc")) "--no-cache" else a;
    return out;
}

pub fn main(init: std.process.Init) !void {
    const alloc = init.arena.allocator();
    const io = init.io;

    var stdout_buffer: [4096]u8 = undefined;
    var stdout_writer: std.Io.File.Writer = .init(.stdout(), io, &stdout_buffer);
    const out = &stdout_writer.interface;

    var it = std.process.Args.Iterator.init(init.minimal.args);
    const raw_args = try zargs.collectProcessArgs(alloc, &it);
    const args = try remapLegacyNc(alloc, raw_args);

    var diag: Declarative.Diagnostics = .{};
    const cli = Declarative.parseStruct(Cli, alloc, args, &diag) catch |err| {
        try out.print("Error: {t}\n\n", .{err});
        try scrapy.showHelp(out);
        try out.flush();
        std.process.exit(1);
    };

    if (cli.help.value) {
        try scrapy.showHelp(out);
        try out.flush();
        return;
    }
    if (cli.version.value) {
        try scrapy.showVersion(out);
        try out.flush();
        return;
    }

    var scraper = scrapy.Scraper.init(alloc, io, out, cli.format.value, !cli.cache.value) catch {
        out.flush() catch {};
        std.process.exit(1);
    };
    defer scraper.deinit();

    if (cli.send.value.len > 0) try scraper.send(cli.send.value);
    if (cli.output.value.len > 0) try scraper.save(cli.output.value);
    if (!cli.silent.value) try scraper.show();

    try out.flush();
}
