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
const Cli = struct {
    help: bool = false,
    version: bool = false,
    format: []const u8 = "table",
    cache: bool = true,
    send: []const u8 = "",
    output: []const u8 = "",
    silent: bool = false,
};

fn parseCli(args: []const [:0]const u8) !Cli {
    var parser = zargs.Parser.init(args, &.{
        .{ .short = 'h', .long = "help" },
        .{ .short = 'v', .long = "version" },
        .{ .short = 'f', .long = "format", .kind = .value },
        .{ .long = "cache" },
        .{ .long = "no-cache" },
        .{ .short = 's', .long = "send", .kind = .value },
        .{ .short = 'o', .long = "output", .kind = .value },
        .{ .long = "silent" },
    });
    var cli: Cli = .{};
    while (true) {
        const arg_index = parser.arg_index;
        switch (parser.next()) {
            .end => return cli,
            .flag => |flag| {
                const name = flag.long.?;
                if (std.mem.eql(u8, name, "help")) cli.help = true;
                if (std.mem.eql(u8, name, "version")) cli.version = true;
                if (std.mem.eql(u8, name, "cache")) cli.cache = true;
                if (std.mem.eql(u8, name, "no-cache")) cli.cache = false;
                if (std.mem.eql(u8, name, "silent")) cli.silent = true;
            },
            .option => |option| {
                // Preserva los valores adjuntos legacy: -f=json, -o=archivo.
                const arg = args[arg_index];
                const attached = arg.len > 2 and arg[0] == '-' and arg[1] != '-' and std.mem.indexOfScalar(u8, arg, '=') != null;
                const value = if (attached and std.mem.startsWith(u8, option.value, "=")) option.value[1..] else option.value;
                const name = option.long.?;
                if (std.mem.eql(u8, name, "format")) cli.format = value;
                if (std.mem.eql(u8, name, "send")) cli.send = value;
                if (std.mem.eql(u8, name, "output")) cli.output = value;
            },
            .unknown_option => return error.UnknownOption,
            .missing_value => return error.MissingValue,
            .unexpected_value => return error.UnexpectedValue,
            .positional => return error.UnexpectedPositional,
        }
    }
}

/// `-nc` era el short original de `--no-cache`; z-args Simple solo
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

    const process_args = try init.minimal.args.toSlice(alloc);
    const raw_args = process_args[1..];
    const args = try remapLegacyNc(alloc, raw_args);

    const cli = parseCli(args) catch |err| {
        try out.print("Error: {t}\n\n", .{err});
        try scrapy.showHelp(out);
        try out.flush();
        std.process.exit(1);
    };

    if (cli.help) {
        try scrapy.showHelp(out);
        try out.flush();
        return;
    }
    if (cli.version) {
        try scrapy.showVersion(out);
        try out.flush();
        return;
    }

    var scraper = scrapy.Scraper.init(alloc, io, out, cli.format, !cli.cache) catch {
        out.flush() catch {};
        std.process.exit(1);
    };
    defer scraper.deinit();

    if (cli.send.len > 0) try scraper.send(cli.send);
    if (cli.output.len > 0) try scraper.save(cli.output);
    if (!cli.silent) try scraper.show();

    try out.flush();
}

test "CLI: formatos, valores y flags" {
    const cli = try parseCli(&.{ "-f=json", "--output=data.json", "--send", "https://example.com", "--silent", "--no-cache" });
    try std.testing.expectEqualStrings("json", cli.format);
    try std.testing.expectEqualStrings("data.json", cli.output);
    try std.testing.expectEqualStrings("https://example.com", cli.send);
    try std.testing.expect(cli.silent);
    try std.testing.expect(!cli.cache);
    const flags = try parseCli(&.{"-hv"});
    try std.testing.expect(flags.help and flags.version);
    try std.testing.expectEqualStrings("=archivo", (try parseCli(&.{ "--output", "=archivo" })).output);
}

test "scraper: incluir sus tests y declaraciones" {
    std.testing.refAllDecls(scrapy);
}

test "CLI: errores y alias legacy" {
    try std.testing.expectError(error.UnknownOption, parseCli(&.{"--unknown"}));
    try std.testing.expectError(error.MissingValue, parseCli(&.{"-o"}));
    try std.testing.expectError(error.UnexpectedValue, parseCli(&.{"--silent=true"}));
    try std.testing.expectError(error.UnexpectedPositional, parseCli(&.{"archivo"}));
    const args = try remapLegacyNc(std.testing.allocator, &.{"-nc"});
    defer std.testing.allocator.free(args);
    try std.testing.expect(!(try parseCli(args)).cache);
}
