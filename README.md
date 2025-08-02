# Indicadores Económicos de Chile

![Banco Central de Chile](https://www.bcentral.cl/image/layout_set_logo?img_id=7011199&t=1753913999001)


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
  - [FMT](https://fmt.dev/11.1/)
  - [Arg.h](https://github.com/adishavit/argh)

---

## 📄 Instalación y Uso

```sh
mkdir build
cmake -S . -B build -G Ninja
ninja -C build install
./build/indicadores
```
Ejemplo de caclulo

```sh
indicadores --uf=7.8 
```
 1 de Agosto 2025
 -------------------------------
 | UF(7.8)       |     305.556 |
 -------------------------------
 | UF            |   39.173,95 |
 | Dolar         |      976,80 |
 | Euro          |    1.116,22 |
 | Yen           |    150,7367 |
 -------------------------------
 | Oro(onza)     |    3.295,19 |
 | Plata(onza)   |       36,71 |
 | Cobre(Libra)  |        4,38 |
 -------------------------------



📬 Contacto
Para consultas o colaboraciones, contáctame en:
c4rl0sill3sc4@protonmail.com
