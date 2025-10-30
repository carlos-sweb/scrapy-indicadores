# Indicadores Económicos de Chile

![image](https://www.bcentral.cl/image/layout_set_logo?img_id=7708671&t=1761411348608)
[Banco Central de Chile](https://www.bcentral.cl)


## 📌 Propósito del Proyecto
Este proyecto es una herramienta de línea de comandos que obtiene los principales indicadores económicos de Chile directamente desde el sitio web del Banco Central de Chile. Proporciona acceso rápido y sencillo a datos económicos clave como:

- 💵 **Dólar observado**
- 💶 **Euro**
- 🏠 **UF (Unidad de Fomento)**

## 💡 Motivación
Desarrollé este proyecto para experimentar con la librería **lexbor**, un motor HTML y CSS escrito en C que destaca por su:

- ⚡ **Alto rendimiento y eficiencia**
- 📦 **Tamaño compacto**
- 🧠 **Capacidad de análisis robusta de documentos HTML**
- 🌱 **Bajo consumo de recursos**

Mi objetivo era crear una solución ligera pero potente para acceder a información económica crítica sin dependencias pesadas.

---

## 🔮 Características Futuras Planeadas
### 🚀 Exportación de datos a servicios externos
- Envío automático de indicadores a URLs específicas (webhooks)
- Soporte para APIs personalizadas

### 🗄️ Almacenamiento en Base de Datos
- Conexión a bases de datos SQL/NoSQL
- Almacenamiento histórico de indicadores
- Generación de series temporales

### 📊 Funcionalidades Avanzadas
- Alertas personalizadas cuando los indicadores alcanzan ciertos valores
- Exportación a formatos **CSV/JSON**
- Comparativas históricas
- Integración con herramientas de visualización de datos

---

## 🛠️ Instalación y Uso

### **Prerrequisitos**
- Compilador C++ compatible con C++23
- CMake (versión 3.16 o superior)
- Ninja (recomendado para compilaciones más rápidas)
- Librerías:
  - [OpenSSL](https://github.com/openssl/openssl)
  - [CURL](https://curl.se/)
  - [CPR](https://github.com/libcpr/cpr)
  - [LEXBOR](https://lexbor.com/)
  - [C_PRINT](https://github.com/carlos-sweb/c_print)
  - [Arg.h](https://github.com/adishavit/argh)

---

## 📄 Instalación y Uso
build.sh
```sh
#!/bin/bash
if [ ! -d build ]; then
  mkdir build
fi
cmake -S . -B build -G Ninja
ninja -C build install
./build/indicadores "$@"
```
Modo de uso

```sh
indicadores --help
```
```sh
Indicadores Chile

 Modo de uso:

 -f,--formato FORMATO  : Tipo de formato de salida
                         table(por defecto),json,txt,none
 -s,--send URL         : Envia la información a la url
                         tipo POST(por defecto)
 -h,--help             : Modo de Uso

```


```sh
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


📬 Contacto
Para consultas o colaboraciones, contáctame en:
c4rl0sill3sc4@protonmail.com
