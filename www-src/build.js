import { readFile, writeFile } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";
import CleanCSS from "clean-css";

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const date = new Date();
const day = date.getDate().toString().padStart(2, '0');
const months = ['Enero', 'Febrero', 'Marzo', 'Abril', 'Mayo', 'Junio', 'Julio', 'Agosto', 'Septiembre', 'Octubre', 'Noviembre', 'Diciembre'];
const month = months[date.getMonth()];
const year = date.getFullYear();


const create_card = (name,value)=>{
	const clpIndicators = ["UF", "Dolar", "Euro"];
	const usdIndicators = ["Oro", "Plata", "Cobre", "Yen"];

	const moneyType = clpIndicators.includes(name) ? "CLP" :
	                  usdIndicators.includes(name) ? "USD" : "CLP";

	return `	<div class="card">
		<div class="header-card">
			<span class="title">${name}</span>
		</div>
		<div class="body-card">
			<span class="price">${value}</span><br>
			<span class="money-type">${moneyType}</span>
		</div>
	</div>`;
}


		
	const html_head = `<!DOCTYPE html>
<html lang="es">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Indicadores Chile</title>	
	<link rel="preconnect" href="https://fonts.googleapis.com">
	<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
	<link href="https://fonts.googleapis.com/css2?family=JetBrains+Mono:ital,wght@0,100..800;1,100..800&display=swap" rel="stylesheet">
	<link rel="stylesheet" href="style.min.css">
</head>
<body>
	<!-- NAVBAR -->
	<section class="navbar">
		<p>Indicadores </p>
		<p>Económicos Chile</p>
	</section>

	<!-- CONTENEDOR PRINCIPAL -->
	<div class="container">
		<!-- FECHA -->
		<div class="card-hidden">
			<div class="header-card-hidden">
				<span class="title">${day} de ${month}, ${year}</span>
			</div>
		</div>
`




const html_body =`
		<!-- CARDS DE INDICADORES -->
		<div class="card">
			<div class="header-card">
				<span class="title">UF</span>
			</div>
			<div class="body-card">
				<span class="price">39.587,48</span><br>
				<span class="money-type">CLP</span>
			</div>
		</div>

		<div class="card">
			<div class="header-card">
				<span class="title">Dólar</span>
			</div>
			<div class="body-card">
				<span class="price">941,63</span><br>
				<span class="money-type">CLP</span>
			</div>
		</div>

		<div class="card">
			<div class="header-card">
				<span class="title">Euro</span>
			</div>
			<div class="body-card">
				<span class="price">1.098,24</span><br>
				<span class="money-type">CLP</span>
			</div>
		</div>

		<div class="card">
			<div class="header-card">
				<span class="title">Yen</span>
			</div>
			<div class="body-card">
				<span class="price">152,1600</span><br>
				<span class="money-type">USD</span>
			</div>
		</div>

		<div class="card">
			<div class="header-card">
				<span class="title">Oro<span class="footer-source">&nbsp;(onza troy)</span></span>

			</div>
			<div class="body-card">
				<span class="price">3.963,54</span><br>
				<span class="money-type">USD</span>
			</div>
		</div>

		<div class="card">
			<div class="header-card">
				<span class="title">Plata<span class="footer-source">&nbsp;(onza troy)</span></span>
			</div>
			<div class="body-card">
				<span class="price">47,20</span><br>
				<span class="money-type">USD</span>
			</div>
		</div>

		<div class="card">
			<div class="header-card">
				<span class="title">Cobre</span>
			</div>
			<div class="body-card">
				<span class="price">4,99</span><br>
				<span class="money-type">USD</span>
			</div>
		</div>
	</div>
`
const html_footer = 	
`
	<!-- FOOTER -->
	<section class="footer">
		<div class="footer-content">
			<div class="footer-source">
				Datos obtenidos desde <a href="https://www.bcentral.cl/" target="_blank">www.bcentral.cl</a>
			</div>
			
			<div class="footer-update">
				Última actualización: ${day} de ${month}, ${year} • Actualización diaria
			</div>
			
			<div class="footer-disclaimer">
				Información referencial. Verificar con fuentes oficiales para transacciones formales.
			</div>
			
			<div class="footer-credit">
				Desarrollado con ☕ en Chile • 2025
			</div>
		</div>
	</section>
</body>
</html>`

// Leer el archivo JSON
const dataPath = join(__dirname, "data.json");
const dataContent = await readFile(dataPath, "utf-8");
const contents = JSON.parse(dataContent);

// Construir el HTML
let htmlOutput = html_head;
for(const name in contents) {
	htmlOutput += create_card(name, contents[name]);
}
htmlOutput += html_footer;

// Escribir el archivo HTML
const outputPath = join(__dirname, "..", "www", "index.html");
await writeFile(outputPath, htmlOutput, "utf-8");

// Minificar CSS
const normalizeCssPath = join(__dirname, "normalize.css");
const styleCssPath = join(__dirname, "style.css");

const normalizeCss = await readFile(normalizeCssPath, "utf-8");
const styleCss = await readFile(styleCssPath, "utf-8");

// Concatenar ambos archivos CSS
const combinedCss = normalizeCss + "\n" + styleCss;

// Minificar con clean-css
const minifier = new CleanCSS();
const minified = minifier.minify(combinedCss);

// Escribir el CSS minificado
const cssOutputPath = join(__dirname, "..", "www", "style.min.css");
await writeFile(cssOutputPath, minified.styles, "utf-8");