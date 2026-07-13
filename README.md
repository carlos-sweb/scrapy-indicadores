# 🇨🇱 Indicadores Económicos de Chile

## 📌 Propósito del Proyecto
Este proyecto es una herramienta de línea de comandos que obtiene los principales indicadores económicos de Chile directamente desde el sitio web del Banco Central de Chile. Proporciona acceso rápido y sencillo a datos económicos clave como:

- 💵 **Dólar observado - CLP**
- 💶 **Euro - CLP**
- 🏠 **UF (Unidad de Fomento) - CLP**
- 🥇 **Oro (Onza troy) - USD**
- 🥈 **Plata (Onza troy) - USD**
- 💴 **Yen - USD**
- ⛏️ **Cobre**

Además del uso en terminal, un workflow de GitHub Actions lo ejecuta a diario y publica los datos en un [sitio estático](https://carlos-sweb.github.io/scrapy-indicadores/).

## 💡 Motivación
Desarrollé este proyecto para experimentar con la librería **lexbor**, un motor HTML y CSS escrito en C que destaca por su:

- ⚡ **Alto rendimiento y eficiencia**
- 📦 **Tamaño compacto**
- 🧠 **Capacidad de análisis robusta de documentos HTML**
- 🌱 **Bajo consumo de recursos**

La versión original estaba escrita en **C++** (cpr, ada, argh, c_print). En julio de 2026 el proyecto fue **migrado a [Zig](https://ziglang.org/)**: hoy es Zig puro + lexbor, donde la librería estándar de Zig reemplazó todo lo demás (`std.http.Client` para HTTP/TLS, `std.Uri` para URLs, `std.Io` para archivos). El binario final solo depende de libc.

## ⚙️ Cómo funciona

1. 🌐 Descarga la página de indicadores diarios del Banco Central (`std.http.Client`, con reintentos ante fallas transitorias)
2. 🔍 Parsea el HTML con **lexbor** y extrae los valores por ID de elemento
3. 🗄️ Mantiene una caché diaria del HTML (`<install-bin-dir>/.scrapy-indicadores/DD-MM-YYYY.html`)
4. 📤 Muestra, guarda (`-o`) o envía por POST (`-s`) los indicadores

---

## 🛠️ Instalación

### **Prerrequisitos**
- [Zig](https://ziglang.org/) 0.16.0 o superior — nada más

La única dependencia externa se resuelve sola al compilar:
- [LEXBOR](https://lexbor.com/) — se descarga automáticamente vía `build.zig.zon` (hash verificado)

### **Compilar**

```sh
# Compilar (el binario queda en zig-out/bin/indicadores)
zig build -Doptimize=ReleaseSafe

# Correr los tests
zig build test

# Compilar y ejecutar directo
zig build run -- -f=json
```

Opciones de build:

```sh
# Directorio base de la caché (por defecto /opt/indicadores/bin)
zig build -Dinstall-bin-dir=$HOME

# Cross-compilación (sin dependencias externas del target)
zig build -Dtarget=aarch64-linux-gnu -Doptimize=ReleaseSafe
zig build -Dtarget=x86_64-linux-gnu.2.31 -Doptimize=ReleaseSafe
```

### **Publicar el binario del CI**

El workflow diario **no compila**: ejecuta el binario musl-estático commiteado en
`bin/indicadores-x86_64` (1.4 MB, sin dependencias — corre en cualquier Linux x86_64).
⚠️ **Después de cambiar el código hay que regenerarlo y commitearlo junto al cambio:**

```sh
zig build -Dtarget=x86_64-linux-musl -Doptimize=ReleaseSmall
cp zig-out/bin/indicadores bin/indicadores-x86_64
git add bin/indicadores-x86_64
```

---

## 📄 Uso

```sh
indicadores --help
```
```sh
 Indicadores Chile
 Version: 0.1.0

 Modo de uso:
 -f,--formato <FORMATO> : Tipo de formato de salida
                          table(por defecto),json,txt,none
 -s,--send <URL>        : Envia la información a la url
                          tipo POST(por defecto)
 -nc,--no-cache         : Remueve el sistema de cache
 -h,--help              : Modo de uso
 -o,--output <PATH>     : Ruta del archivo de salida
 --silent               : Modo silencioso
 -v,--version           : Muestra la Versión
```

### Formato tabla (por defecto)

```sh
indicadores
```
```
+------------------------------+
|    24 de Septiembre 2025     |
+--------------+---------------+
| UF           |     39.485,65 |
+--------------+---------------+
| Dolar        |        952,87 |
+--------------+---------------+
| Euro         |      1.125,39 |
+--------------+---------------+
| Yen          |      147,6267 |
+--------------+---------------+
| Oro          |      3.780,37 |
+--------------+---------------+
| Plata        |         44,27 |
+--------------+---------------+
| Cobre        |          4,49 |
+--------------+---------------+
```

### Otros formatos y salidas

```sh
# JSON por consola (valores normalizados a punto decimal)
indicadores -f=json

# Texto plano clave:valor
indicadores -f=txt

# Guardar en archivo (valores crudos, como aparecen en la página)
indicadores --silent -o=data.json

# Enviar por POST como JSON a un webhook
indicadores -f=none -s=https://mi-servidor.cl/api/indicadores
```

---

## 🔮 Características Futuras Planeadas

### 🗄️ Almacenamiento en Base de Datos
- Conexión a bases de datos SQL/NoSQL
- Almacenamiento histórico de indicadores
- Generación de series temporales

### 📊 Funcionalidades Avanzadas
- Alertas personalizadas cuando los indicadores alcanzan ciertos valores
- Exportación a formato **CSV**
- Comparativas históricas
- Integración con herramientas de visualización de datos

---

## 🌐 Fuente

[Banco Central de Chile](https://www.bcentral.cl)

## 👀 Ejemplo online

[Sitio web desplegado](https://carlos-sweb.github.io/scrapy-indicadores/) — actualizado a diario por GitHub Actions

📬 Contacto
Para consultas o colaboraciones, contáctame en:
c4rl0sill3sc4@protonmail.com
