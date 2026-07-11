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

const Options = struct {
    help: bool = false,
    version: bool = false,
    silent: bool = false,
    nc: bool = false,
    format: []const u8 = "table",
    send: ?[]const u8 = null,
    output: ?[]const u8 = null,
};

/// Acepta `-x valor`, `-x=valor`, `--largo valor` y `--largo=valor`.
fn takeValue(i: *usize, args: []const [:0]const u8, short: []const u8, long: []const u8) ?[]const u8 {
    const arg = args[i.*];
    for ([2][]const u8{ short, long }) |name| {
        if (std.mem.eql(u8, arg, name)) {
            if (i.* + 1 < args.len) {
                i.* += 1;
                return args[i.*];
            }
            return null; // sin valor: se ignora, como argh
        }
        if (arg.len > name.len + 1 and std.mem.startsWith(u8, arg, name) and arg[name.len] == '=') {
            return arg[name.len + 1 ..];
        }
    }
    return null;
}

fn isFlag(arg: []const u8, short: []const u8, long: []const u8) bool {
    return std.mem.eql(u8, arg, short) or std.mem.eql(u8, arg, long);
}

fn parseArgs(args: []const [:0]const u8) Options {
    var opts = Options{};
    var i: usize = 1;
    while (i < args.len) : (i += 1) {
        const arg = args[i];
        if (isFlag(arg, "-h", "--help")) {
            opts.help = true;
        } else if (isFlag(arg, "-v", "--version")) {
            opts.version = true;
        } else if (isFlag(arg, "-nc", "--no-cache")) {
            opts.nc = true;
        } else if (std.mem.eql(u8, arg, "--silent")) {
            opts.silent = true;
        } else if (takeValue(&i, args, "-f", "--format")) |v| {
            opts.format = v;
        } else if (takeValue(&i, args, "-s", "--send")) |v| {
            opts.send = v;
        } else if (takeValue(&i, args, "-o", "--output")) |v| {
            opts.output = v;
        }
    }
    return opts;
}

pub fn main(init: std.process.Init) !void {
    const alloc = init.arena.allocator();
    const io = init.io;

    var stdout_buffer: [4096]u8 = undefined;
    var stdout_writer: std.Io.File.Writer = .init(.stdout(), io, &stdout_buffer);
    const out = &stdout_writer.interface;

    const args = try init.minimal.args.toSlice(alloc);
    const opts = parseArgs(args);

    if (opts.help) {
        try scrapy.showHelp(out);
        try out.flush();
        return;
    }
    if (opts.version) {
        try scrapy.showVersion(out);
        try out.flush();
        return;
    }

    var scraper = scrapy.Scraper.init(alloc, io, out, opts.format, opts.nc) catch {
        out.flush() catch {};
        std.process.exit(1);
    };
    defer scraper.deinit();

    if (opts.send) |url| try scraper.send(url);
    if (opts.output) |path| try scraper.save(path);
    if (!opts.silent) try scraper.show();

    try out.flush();
}
