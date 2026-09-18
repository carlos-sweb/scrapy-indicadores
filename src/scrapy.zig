//! Port a Zig de scrapycpp.cpp: scraper de indicadores economicos
//! del Banco Central de Chile (z-lexbor + std.http.Client).
const std = @import("std");
const lexbor = @import("z_lexbor");
const build_options = @import("build_options");

const Allocator = std.mem.Allocator;
const Io = std.Io;
const Writer = std.Io.Writer;

pub const version = "0.2.0";
pub const url_central = "https://si3.bcentral.cl/Indicadoressiete/secure/Indicadoresdiarios.aspx";

pub const months_es = [12][]const u8{
    "Enero", "Febrero", "Marzo",      "Abril",   "Mayo",      "Junio",
    "Julio", "Agosto",  "Septiembre", "Octubre", "Noviembre", "Diciembre",
};

pub const accepted_formats = [4][]const u8{ "table", "json", "txt", "none" };

const red = "\x1b[31m";
const green = "\x1b[32m";
const yellow = "\x1b[33m";
const reset = "\x1b[0m";

pub const Indicator = struct { name: []const u8, id: []const u8 };

pub const target_indicators = [_]Indicator{
    .{ .name = "UF", .id = "lblValor1_1" },
    .{ .name = "Dolar", .id = "lblValor1_3" },
    .{ .name = "Euro", .id = "lblValor1_5" },
    .{ .name = "Yen", .id = "lblValor1_10" },
    .{ .name = "Oro", .id = "lblValor2_3" },
    .{ .name = "Plata", .id = "lblValor2_4" },
    .{ .name = "Cobre", .id = "lblValor2_5" },
};

pub fn showVersion(out: *Writer) !void {
    try out.print(" Version: " ++ yellow ++ "{s}" ++ reset ++ "\n", .{version});
}

pub fn showHelp(out: *Writer) !void {
    try out.print(" " ++ green ++ "{s}" ++ reset ++ "\n", .{"Indicadores Chile"});
    try showVersion(out);
    try out.writeAll("\n Modo de uso:\n");
    try out.print("{s:<24}: {s}\n{s:>26}{s}\n", .{ " -f,--format <FORMATO>", "Tipo de formato de salida", "", "table(por defecto),json,txt,none" });
    try out.print("{s:<24}: {s}\n{s:>26}{s}\n", .{ " -s,--send <URL>", "Envia la información a la url", "", "tipo POST(por defecto)" });
    try out.print("{s:<24}: {s}\n", .{ " -nc,--no-cache", "Remueve el sistema de cache" });
    try out.print("{s:<24}: {s}\n", .{ " -h,--help", "Modo de uso" });
    try out.print("{s:<24}: {s}\n", .{ " -o,--output <PATH>", "Ruta del archivo de salida" });
    try out.print("{s:<24}: {s}\n", .{ " --silent", "Modo silencioso" });
    try out.print("{s:<24}: {s}\n\n", .{ " -v,--version", "Muestra la Versión" });
}

pub fn toLowercase(alloc: Allocator, str: []const u8) ![]const u8 {
    const out = try alloc.alloc(u8, str.len);
    for (str, 0..) |ch, i| out[i] = std.ascii.toLower(ch);
    return out;
}

pub fn toUppercase(alloc: Allocator, str: []const u8) ![]const u8 {
    const out = try alloc.alloc(u8, str.len);
    for (str, 0..) |ch, i| out[i] = std.ascii.toUpper(ch);
    return out;
}

/// Limpia valores en formato chileno: "39.485,65" -> "39485.65".
/// "ND" (sin datos, fines de semana/feriados) o vacio -> "0.0".
pub fn cleanValue(alloc: Allocator, value: []const u8) ![]const u8 {
    if (value.len == 0 or std.mem.eql(u8, value, "ND"))
        return alloc.dupe(u8, "0.0");

    var out: std.ArrayList(u8) = .empty;
    try out.ensureTotalCapacity(alloc, value.len);
    for (value) |ch| {
        if (ch == '.') continue; // separador de miles
        out.appendAssumeCapacity(if (ch == ',') '.' else ch);
    }
    return out.toOwnedSlice(alloc);
}

/// Fecha actual en espanol: "DD de Mes YYYY".
pub fn getDateText(alloc: Allocator, io: Io) ![]const u8 {
    const ts = std.Io.Clock.real.now(io);
    const epoch_seconds = std.time.epoch.EpochSeconds{ .secs = @intCast(ts.toSeconds()) };
    const epoch_day = epoch_seconds.getEpochDay();
    const year_and_day = epoch_day.calculateYearDay();
    const month_and_day = year_and_day.calculateMonthDay();

    return std.fmt.allocPrint(alloc, "{d:0>2} de {s} {d}", .{
        month_and_day.day_index + 1, // day_index es 0-based
        months_es[@intCast(month_and_day.month.numeric() - 1)],
        year_and_day.year,
    });
}

pub fn isFormatAccepted(format: []const u8) bool {
    for (accepted_formats) |accepted| {
        if (std.mem.eql(u8, format, accepted)) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// DOM (z-lexbor wrapper)
// ---------------------------------------------------------------------------

pub const Dom = struct {
    parser: lexbor.html.Parser,
    engine: lexbor.selectors.Engine,
    doc: lexbor.html.Document,

    /// Parsea un HTML completo. El parser y el engine de selectores son RAII:
    /// `deinit` los libera a ambos (y el documento con ellos).
    pub fn parse(html: []const u8) !Dom {
        var parser = try lexbor.html.Parser.createInit();
        errdefer parser.deinit();

        var engine = try lexbor.selectors.Engine.createInit();
        errdefer engine.deinit();

        const doc = try parser.parse(html);
        // Verificar que el documento tiene un nodo raiz (equivalente al check
        // de body != null del codigo original).
        _ = doc.rootNode() orelse return error.NoBody;

        return .{ .parser = parser, .engine = engine, .doc = doc };
    }

    pub fn deinit(self: *Dom) void {
        self.engine.deinit();
        self.parser.deinit();
    }

    /// Texto del elemento con el atributo id dado, o null si no existe.
    /// Usa el motor de selectores CSS de lexbor (querySelector sobre `#id`).
    pub fn getElementById(self: *Dom, alloc: Allocator, id: []const u8) !?[]const u8 {
        const root = self.doc.rootNode() orelse return null;

        // Selector CSS "#elId" — los IDs tienen max 15 chars, 32 bytes alcanza.
        var selector_buf: [32]u8 = undefined;
        const selector = std.fmt.bufPrint(&selector_buf, "#{s}", .{id}) catch return null;

        const node = (self.engine.queryFirst(root, selector) catch return null) orelse return null;
        const text = node.textContent();
        return try alloc.dupe(u8, text);
    }
};

// ---------------------------------------------------------------------------
// HTTP (std.http.Client)
//
// Nota DNS: en Linux, Zig 0.16 usa SIEMPRE su resolver propio (nunca
// getaddrinfo, aunque se linkee libc) y este falla de forma intermitente
// con la respuesta CNAME->GSLB de si3.bcentral.cl. Workaround: fijar el
// host en /etc/hosts (Zig lo consulta antes de hacer DNS); el workflow de
// CI lo hace automaticamente. Los reintentos de httpGet mitigan el resto.
// ---------------------------------------------------------------------------

pub const HttpResponse = struct { status: u16, body: []u8 };

/// GET con redirects y descompresion automaticos, con reintentos ante
/// fallas transitorias. Si todos fallan devuelve status 0 y body vacio
/// (semantica de cpr).
pub fn httpGet(alloc: Allocator, io: Io, url: []const u8) !HttpResponse {
    var client: std.http.Client = .{ .allocator = alloc, .io = io };
    defer client.deinit();

    // El GSLB del Banco Central rota IPs con TTL de 5s y a veces entrega
    // un backend muerto: los reintentos espaciados cruzan varias ventanas.
    const max_attempts = 4;
    var attempt: u8 = 0;
    while (attempt < max_attempts) : (attempt += 1) {
        if (attempt != 0) io.sleep(.fromSeconds(5), .awake) catch {};

        var body: std.Io.Writer.Allocating = .init(alloc);
        const result = client.fetch(.{
            .location = .{ .url = url },
            .response_writer = &body.writer,
        }) catch |err| {
            std.debug.print("Aviso: intento {d}/{d} fallo ({t})\n", .{ attempt + 1, max_attempts, err });
            body.deinit();
            continue;
        };
        return .{ .status = @intFromEnum(result.status), .body = try body.toOwnedSlice() };
    }
    return .{ .status = 0, .body = "" };
}

/// POST JSON. Devuelve el codigo HTTP, o 0 si fallo la conexion (como cpr).
pub fn httpPostJson(alloc: Allocator, io: Io, url: []const u8, payload: []const u8) u16 {
    var client: std.http.Client = .{ .allocator = alloc, .io = io };
    defer client.deinit();

    const result = client.fetch(.{
        .location = .{ .url = url },
        .payload = payload,
        .extra_headers = &.{.{ .name = "Content-Type", .value = "application/json" }},
    }) catch return 0;

    return @intFromEnum(result.status);
}

// ---------------------------------------------------------------------------
// Scraper (equivalente a HtmlDom)
// ---------------------------------------------------------------------------

pub const Scraper = struct {
    alloc: Allocator,
    io: Io,
    out: *Writer,
    format: []const u8,
    /// Flag -nc/--no-cache. Se replica el comportamiento del original:
    /// sin -nc se descarga siempre (y se actualiza la cache); con -nc se
    /// lee la cache del dia si existe.
    nc_flag: bool,
    dom: Dom,
    values: std.ArrayList(Value),

    pub const Value = struct { name: []const u8, value: []const u8 };

    pub fn init(alloc: Allocator, io: Io, out: *Writer, format: []const u8, nc_flag: bool) !Scraper {
        if (!isFormatAccepted(format)) {
            try out.print("\n " ++ red ++ "{s}" ++ reset ++ ": El formato \"" ++ yellow ++ "{s}" ++ reset ++ "\" no es válido\n" ++
                " * table (por defecto)\n * json\n * txt\n\n", .{ "Error", format });
            return error.InvalidFormat;
        }

        const html = try loadContentFromBCentral(alloc, io, out, nc_flag);

        var dom = Dom.parse(html) catch |err| {
            const msg = switch (err) {
                error.OutOfMemory => "No se pudo crear el documento HTML",
                error.LexborError => "Error al parsear el documento HTML",
                error.NoBody => "No se pudo obtener el body del documento",
                else => "Error inesperado en el parser",
            };
            try out.print(red ++ "{s}" ++ reset ++ ": {s}\n", .{ "Error", msg });
            return err;
        };
        errdefer dom.deinit();

        var self = Scraper{
            .alloc = alloc,
            .io = io,
            .out = out,
            .format = format,
            .nc_flag = nc_flag,
            .dom = dom,
            .values = .empty,
        };
        try self.parseIndicators();
        return self;
    }

    pub fn deinit(self: *Scraper) void {
        self.dom.deinit();
    }

    fn parseIndicators(self: *Scraper) !void {
        for (target_indicators) |ind| {
            const result = (try self.dom.getElementById(self.alloc, ind.id)) orelse {
                try self.out.print(red ++ "{s}" ++ reset ++ ": No se pudo encontrar el elemento con ID '" ++
                    yellow ++ "{s}" ++ reset ++ "'\n", .{ "Error", ind.id });
                return error.ElementNotFound;
            };
            if (!std.mem.eql(u8, result, "ND")) {
                try self.values.append(self.alloc, .{ .name = ind.name, .value = result });
            }
        }
    }

    pub fn show(self: *Scraper) !void {
        if (std.mem.eql(u8, self.format, "table")) {
            try self.showTableFormat();
        } else if (std.mem.eql(u8, self.format, "json")) {
            try self.showJsonFormat();
        } else if (std.mem.eql(u8, self.format, "txt")) {
            try self.showTxtFormat();
        }
    }

    fn showTxtFormat(self: *Scraper) !void {
        for (self.values.items) |v| {
            try self.out.print("{s}:{s}\n", .{
                try toLowercase(self.alloc, v.name),
                try cleanValue(self.alloc, v.value),
            });
        }
    }

    fn showJsonFormat(self: *Scraper) !void {
        try self.out.writeAll("{\n");
        for (self.values.items, 0..) |v, i| {
            try self.out.print(" \"{s}\":{s}{s}\n", .{
                try toLowercase(self.alloc, v.name),
                try cleanValue(self.alloc, v.value),
                if (i == self.values.items.len - 1) "" else ",",
            });
        }
        try self.out.writeAll("}\n");
    }

    fn showTableFormat(self: *Scraper) !void {
        try self.out.print("+{s:-^30}+\n", .{"+"});
        try self.out.print("|" ++ green ++ "{s:^30}" ++ reset ++ "|\n", .{try getDateText(self.alloc, self.io)});
        try self.out.print("+{s:-^30}+\n", .{"+"});
        for (self.values.items) |v| {
            try self.out.print("| " ++ green ++ "{s:<13}" ++ reset ++ "|" ++ yellow ++ "{s:>14}" ++ reset ++ " |\n", .{ v.name, v.value });
            try self.out.print("+{s:-^30}+\n", .{"+"});
        }
    }

    /// Guarda los indicadores (nombre y valor crudos) como JSON en `path`.
    pub fn save(self: *Scraper, path: []const u8) !void {
        var buf: std.ArrayList(u8) = .empty;
        defer buf.deinit(self.alloc);

        try buf.appendSlice(self.alloc, "{\n");
        for (self.values.items, 0..) |v, i| {
            try buf.appendSlice(self.alloc, "\t\"");
            try buf.appendSlice(self.alloc, v.name);
            try buf.appendSlice(self.alloc, "\":\"");
            try buf.appendSlice(self.alloc, v.value);
            try buf.appendSlice(self.alloc, "\"");
            if (i < self.values.items.len - 1) try buf.appendSlice(self.alloc, ",");
            try buf.appendSlice(self.alloc, "\n");
        }
        try buf.appendSlice(self.alloc, "}");

        Io.Dir.cwd().writeFile(self.io, .{ .sub_path = path, .data = buf.items }) catch {
            try self.out.print(red ++ "{s}" ++ reset ++ ": No se pudo crear el archivo '" ++
                yellow ++ "{s}" ++ reset ++ "'\n", .{ "Error", path });
            return;
        };
    }

    /// Envia los indicadores (limpios, en minusculas) como JSON via POST.
    pub fn send(self: *Scraper, url: []const u8) !void {
        _ = std.Uri.parse(url) catch {
            if (url.len != 0) {
                try self.out.print(red ++ "{s}" ++ reset ++ ": La URL \"" ++
                    yellow ++ "{s}" ++ reset ++ "\" no es válida\n", .{ "Error", url });
            }
            return;
        };

        var body: std.ArrayList(u8) = .empty;
        defer body.deinit(self.alloc);
        try body.appendSlice(self.alloc, "{");
        for (self.values.items, 0..) |v, i| {
            try body.appendSlice(self.alloc, "\"");
            try body.appendSlice(self.alloc, try toLowercase(self.alloc, v.name));
            try body.appendSlice(self.alloc, "\":");
            try body.appendSlice(self.alloc, try cleanValue(self.alloc, v.value));
            if (i != self.values.items.len - 1) try body.appendSlice(self.alloc, ",");
        }
        try body.appendSlice(self.alloc, "}");

        const status = httpPostJson(self.alloc, self.io, url, body.items);
        if (status == 0) {
            try self.out.print(red ++ "{s}" ++ reset ++ ": No se pudo enviar la información a la URL -> " ++
                yellow ++ "{s}" ++ reset ++ "\n", .{ "Error", url });
        }
    }

    /// Carga el HTML del Banco Central, usando la cache diaria en
    /// INSTALL_BIN_DIR/.scrapy-indicadores (paridad con el original).
    fn loadContentFromBCentral(alloc: Allocator, io: Io, out: *Writer, nc_flag: bool) ![]const u8 {
        const ts = std.Io.Clock.real.now(io);
        const epoch_seconds = std.time.epoch.EpochSeconds{ .secs = @intCast(ts.toSeconds()) };
        const epoch_day = epoch_seconds.getEpochDay();
        const year_and_day = epoch_day.calculateYearDay();
        const month_and_day = year_and_day.calculateMonthDay();

        const cache_dir = build_options.install_bin_dir ++ "/.scrapy-indicadores";
        const cache_path = try std.fmt.allocPrint(alloc, cache_dir ++ "/{d:0>2}-{d:0>2}-{d:0>4}.html", .{
            month_and_day.day_index + 1,
            month_and_day.month.numeric(),
            year_and_day.year,
        });

        const cwd = Io.Dir.cwd();
        cwd.createDirPath(io, cache_dir) catch {
            try out.print(red ++ "{s}" ++ reset ++ ": No se pudo crear el directorio de caché\n", .{"Error"});
        };

        const cache_exists = blk: {
            cwd.access(io, cache_path, .{}) catch break :blk false;
            break :blk true;
        };

        if (!cache_exists or !nc_flag) {
            const resp = try httpGet(alloc, io, url_central);
            const html: []const u8 = if (resp.status == 200) resp.body else "";
            if (html.len != 0) {
                cwd.writeFile(io, .{ .sub_path = cache_path, .data = html }) catch {
                    try out.print(red ++ "{s}" ++ reset ++ ": No se pudo crear el archivo de caché\n", .{"Error"});
                };
            }
            return html;
        }

        const raw = cwd.readFileAlloc(io, cache_path, alloc, .unlimited) catch {
            try out.print(red ++ "{s}" ++ reset ++ ": Error al abrir el archivo de caché\n", .{"Error"});
            return "";
        };

        // El original leia linea a linea descartando los '\n'.
        var html: std.ArrayList(u8) = .empty;
        try html.ensureTotalCapacity(alloc, raw.len);
        for (raw) |ch| {
            if (ch != '\n') html.appendAssumeCapacity(ch);
        }
        return html.toOwnedSlice(alloc);
    }
};

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

test "cleanValue: formato chileno a numerico" {
    const a = std.testing.allocator;
    const v = try cleanValue(a, "39.485,65");
    defer a.free(v);
    try std.testing.expectEqualStrings("39485.65", v);
}

test "cleanValue: ND y vacio devuelven 0.0" {
    const a = std.testing.allocator;
    const nd = try cleanValue(a, "ND");
    defer a.free(nd);
    try std.testing.expectEqualStrings("0.0", nd);

    const empty = try cleanValue(a, "");
    defer a.free(empty);
    try std.testing.expectEqualStrings("0.0", empty);
}

test "cleanValue: sin separador de miles" {
    const a = std.testing.allocator;
    const v = try cleanValue(a, "985,12");
    defer a.free(v);
    try std.testing.expectEqualStrings("985.12", v);
}

test "toLowercase" {
    const a = std.testing.allocator;
    const v = try toLowercase(a, "Dolar");
    defer a.free(v);
    try std.testing.expectEqualStrings("dolar", v);
}

test "isFormatAccepted" {
    try std.testing.expect(isFormatAccepted("table"));
    try std.testing.expect(isFormatAccepted("json"));
    try std.testing.expect(isFormatAccepted("txt"));
    try std.testing.expect(isFormatAccepted("none"));
    try std.testing.expect(!isFormatAccepted("xml"));
}

test "Dom: getElementById extrae el texto" {
    const a = std.testing.allocator;
    var dom = try Dom.parse("<html><body><div id=\"lblValor1_1\">39.485,65</div></body></html>");
    defer dom.deinit();

    const text = (try dom.getElementById(a, "lblValor1_1")) orelse return error.TestUnexpectedResult;
    defer a.free(text);
    try std.testing.expectEqualStrings("39.485,65", text);

    try std.testing.expectEqual(@as(?[]const u8, null), try dom.getElementById(a, "no_existe"));
}

test "std.Uri: validacion de URLs para send()" {
    _ = try std.Uri.parse("https://example.com/hook");
    _ = try std.Uri.parse("http://192.168.1.10:8080/api?x=1");
    try std.testing.expectError(error.InvalidFormat, std.Uri.parse("no-es-url"));
    try std.testing.expectError(error.InvalidFormat, std.Uri.parse(""));
}
