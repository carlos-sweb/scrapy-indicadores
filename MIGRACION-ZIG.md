# Plan de migración C++ → Zig

**Estado: EJECUTADA Y COMPLETADA — 2026-07-11.** El port vive en `src/main.zig` +
`src/scrapy.zig`. Composición final: **Zig puro + lexbor (C)** — la única dependencia,
v2.7.0 vía `build.zig.zon`. Paridad byte a byte verificada contra `bin/indicadores-x86_64`
en table/json/txt y `-o` (incluidos mensajes de error). `zig build test` cubre lógica + DOM.

Evolución posterior al plan original:
- **libcurl eliminado**: HTTP/TLS vía `std.http.Client` (ver notas abajo).
- **ada eliminado**: su único consumidor era la validación de URL en `send()`;
  reemplazado por `std.Uri.parse`. Con esto desapareció el último componente C++
  del proyecto (y `link_libcpp`). Trade-off aceptado: URLs IDN (dominios unicode)
  ya no se convierten a punycode.

Notas de la ejecución:
- Zig 0.16 deprecó `@cImport`: se usa `b.addTranslateC` sobre `src/c.h`, importado como módulo `"c"`.
- Zig 0.16 movió el filesystem a `std.Io` (`main(init: std.process.Init)`, `Io.Dir.cwd()`,
  `createDirPath`, `readFileAlloc`, `Io.File.Writer`); `std.fs` quedó vacío.
- El flag `-nc/--no-cache` del original está invertido respecto a su nombre (sin `-nc`
  siempre descarga; con `-nc` usa la caché diaria si existe). Se replicó tal cual.
- **libcurl eliminado (2026-07-11)**: HTTP/TLS via `std.http.Client`. El binario solo depende
  de libc. Requisito: linkear libc para que el DNS use `getaddrinfo` (el resolver propio de
  Zig falla con la respuesta CNAME→GSLB de si3.bcentral.cl). El GSLB del sitio (TTL 5s) a
  veces entrega backends muertos → `httpGet` reintenta 4 veces con 5s de espera.
- Cross-compile completo desbloqueado: `zig build -Dtarget=aarch64-linux-gnu` (Termux) y
  `-Dtarget=x86_64-linux-gnu.2.31` (glibc mínima 2.28) compilan sin dependencias externas.
- La caché usa `-Dinstall-bin-dir` (default `/opt/indicadores/bin`, paridad con CMake).

---

Plan original (referencia):

**Estado: viable.** Las tres dependencias críticas fueron validadas con pruebas de concepto
reales (compiladas y ejecutadas con Zig 0.16.0 en esta máquina, 2026-07-11).

## 1. Análisis de dependencias

| Dependencia | Lenguaje real | Veredicto | Estrategia |
|---|---|---|---|
| **lexbor** | C puro (0 símbolos C++ en `liblexbor_static.a`) | ✅ Validado | `@cImport` directo + link estático. Cero fricción. |
| **ada** | C++ interno, **pero expone API C oficial** | ✅ Validado | Usar `ada_c.h` (ya instalado en `/usr/local/include`). No se necesita el `.so` de C++. |
| **cpr** | C++ (solo es un wrapper de libcurl) | ❌ Eliminar | Usar **libcurl directo** (API C). Validado. |
| **argh** | C++ header-only | ❌ Eliminar | Parseo manual con `std.process.argsAlloc` (~40 líneas). |
| **c_print** | C | ⚠️ Opcional | Reemplazar por `std.fmt` + códigos ANSI (recomendado), o mantener el `.a`. |
| `std::string/vector/filesystem/fstream` | C++ stdlib | ❌ Eliminar | `std.ArrayList`, `std.fs`, allocators de Zig. |
| curl / openssl | C sistema | ✅ | Vía libcurl. |

### La respuesta a tu duda sobre ada.h

No necesitas migrar ada ni conseguir builds especiales. Tu `libada.a` **ya exporta la API C**
con linkage C (`ada_parse`, `ada_is_valid`, `ada_get_href`, `ada_free`, `ada_get_protocol`, etc. —
verificado con `nm`). El header es `/usr/local/include/ada_c.h`, que es C puro y `@cImport` lo
traga sin problemas.

Lo único que arrastra el C++ interno es el **runtime**: como `libada.a` fue compilada con GCC,
necesita `libstdc++` y `libgcc_s` del sistema al momento de linkear. El detalle no obvio:

> `-lstdc++` en Zig linkea la **libc++ de LLVM embebida** en Zig, que NO tiene los símbolos
> internos de GNU (`std::__throw_out_of_range_fmt`, etc.). Hay que pasar los `.so` del sistema
> por ruta directa.

Comando validado (compiló y ejecutó correctamente):

```bash
zig build-exe src/main.zig \
  -I/usr/local/include -L/usr/local/lib64 \
  -llexbor_static -lada -lcurl -lc \
  /usr/lib64/libstdc++.so.6 /usr/lib64/libgcc_s.so.1
```

## 2. Dos opciones de arquitectura de build

### Opción A — Linkear las `.a` existentes (arranque rápido)

Lo del comando de arriba, expresado en `build.zig`. Funciona hoy mismo.
Limitación: solo compila para x86_64-linux (las `.a` son de esta máquina) y el binario
depende de `libstdc++.so.6` del sistema.

### Opción B — Compilar ada y lexbor dentro de `build.zig` (recomendada a mediano plazo)

`build.zig` usa clang por debajo y **puede compilar C++**. Ada distribuye un *amalgamation*
de 2 archivos (`ada.cpp` + `ada.h` + `ada_c.h`, se genera con `python singleheader/amalgamate.py`
o se descarga del release de GitHub). Entonces:

```zig
exe.addCSourceFile(.{ .file = b.path("vendor/ada/ada.cpp"), .flags = &.{"-std=c++20"} });
exe.linkLibCpp(); // libc++ de Zig, estática
// lexbor: addCSourceFiles con sus fuentes C, o mantener la .a por target
```

Ventajas: binario sin dependencia de libstdc++ del sistema, y **cross-compilación real**
(`-Dtarget=aarch64-linux-musl`, o aarch64 para Termux) — reemplazaría toda la detección
de Termux del CMakeLists actual.

Sugerencia: empezar con A (fase 1) y pasar a B cuando el port funcione.

## 3. Fases de migración

El código a migrar es pequeño: ~430 líneas de C++ (`main.cpp` + `scrapycpp.cpp`).
Estimado final en Zig: ~500 líneas.

1. **Infraestructura** — `build.zig` con opción A. Un módulo `src/c.zig` con un único
   `@cImport` compartido (`ada_c.h`, `lexbor/html/html.h`, `curl/curl.h`).
2. **Utilidades puras + tests** — `cleanValue` (formato chileno "39.485,65" → "39485.65",
   caso "ND"), `toLowercase`, `getDateText` (meses en español). Cubrir con `zig test`
   antes de seguir: son la lógica de negocio más fina.
3. **HTTP GET con libcurl** — `curl_easy_perform` con write-callback que acumula en un
   `ArrayList(u8)`. (Alternativa: `std.http.Client` de Zig sin dependencia de curl;
   dejarlo como experimento posterior, curl es lo probado contra el sitio del Banco Central.)
4. **Caché en disco** — `std.fs`: crear `.scrapy-indicadores/DD-MM-YYYY.html`, replicando
   `loadContentFromBCentral()`.
5. **Parsing DOM con lexbor** — `getElementById` + `getText` (el PoC ya resolvió los casts:
   `@ptrCast` de `doc.body` a `*lxb_dom_element_t`, colecciones, `lxb_dom_node_text_content`).
   Iterar los 7 indicadores (UF, Dólar, Euro, Yen, Oro, Plata, Cobre).
6. **CLI** — flags `-f/--format`, `-nc/--no-cache`, `-s/--send`, `-o/--output`, `--silent`,
   `-h`, `-v`. Parseo manual, sin librería.
7. **Salidas** — formatos `table` (con colores ANSI verde/amarillo), `json`, `txt`, `none`,
   y `save()` a archivo. Reemplaza c_print.
8. **send() POST** — validar URL con `ada_parse`/`ada_is_valid`, armar el JSON y postear
   con curl (`CURLOPT_POSTFIELDS` + header `Content-Type: application/json`).
9. **Paridad y cierre** — comparar byte a byte la salida de `bin/indicadores-x86_64` vs el
   binario Zig en los 4 formatos (usando el mismo HTML cacheado). Actualizar
   `.github/workflows` y `build.sh`. Retirar los `.cpp` cuando haya paridad.

## 4. Diferencias de diseño C++ → Zig a tener en cuenta

- **Errores**: el código actual hace `exit(EXIT_FAILURE)` dentro del constructor; en Zig
  se vuelve `!HtmlDom` con error sets (`error.FetchFailed`, `error.ElementNotFound`...) y
  los mensajes se imprimen en `main`. Queda más limpio que el original.
- **Memoria**: los strings que hoy son `std::string` pasan a slices + allocator explícito
  (un `ArenaAllocator` por ejecución simplifica todo: se libera de una vez al final).
- **RAII → defer**: `lxb_html_document_destroy`, `curl_easy_cleanup`, `ada_free` con `defer`.
- **Strings de ada**: `ada_get_href` devuelve `ada_string { data, length }` — se convierte
  con `href.data[0..href.length]`, sin copia.

## 5. Riesgos conocidos

| Riesgo | Mitigación |
|---|---|
| `libada.a` compilada con GCC exige libstdc++ del sistema (opción A) | Ya resuelto pasando los `.so` por ruta; desaparece con opción B. |
| Cross-compile a Termux/aarch64 con opción A | Imposible con las `.a` actuales; es exactamente lo que la opción B arregla. |
| `@cImport` con headers de lexbor (macros complejas) | PoC ya validado: parse + by-attr + text_content funcionan. |
| Sitio del Banco Central (TLS/redirects) con `std.http.Client` | No arriesgar: quedarse con libcurl, que es lo que cpr usaba por debajo. |

## Evidencia de los PoC (2026-07-11, Zig 0.16.0)

```
$ ./poc_ada
ada OK -> https://si3.bcentral.cl/Indicadoressiete/secure/Indicadoresdiarios.aspx?a=1

$ ./poc_lexbor
lexbor OK -> 39.485,65
curl OK -> libcurl/8.18.0 OpenSSL/3.5.7 ...
```
