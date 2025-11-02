import { appendFile } from "node:fs/promises";

const date = new Date();
const day = date.getDate().toString().padStart(2, '0');
const months = ['Enero', 'Febrero', 'Marzo', 'Abril', 'Mayo', 'Junio', 'Julio', 'Agosto', 'Septiembre', 'Octubre', 'Noviembre', 'Diciembre'];
const month = months[date.getMonth()];
const year = date.getFullYear();


const create_card = (name,value)=>{
	return `	<div class="card">
		<div class="header-card">
			<span class="title">${name}</span>
		</div>
		<div class="body-card">
			<span class="price">${value}</span><br>
			<span class="money-type">CLP</span>
		</div>
	</div>`;
}


		
	const html_head = `<!DOCTYPE html>
<html lang="es">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Indicadores Chile</title>
	<link rel="manifest" href="manifest.json">
	<link rel="preconnect" href="https://fonts.googleapis.com">
	<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
	<link href="https://fonts.googleapis.com/css2?family=JetBrains+Mono:ital,wght@0,100..800;1,100..800&display=swap" rel="stylesheet">
	<link rel="stylesheet" href="style.min.css">
	<script>
  if ('serviceWorker' in navigator) {
    window.addEventListener('load', () => {
      navigator.serviceWorker.register('/scrapy-indicadores/sw.js', {scope: '/scrapy-indicadores/'})
        .then(registration => {
          console.log('Service Worker registrado con éxito:', registration);
        })
        .catch(error => {
          console.log('Fallo el registro del Service Worker:', error);
        });
    });
  }
</script>
<script src="sw.js"></script>
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

const path = __dirname+"/data.json",
file = Bun.file(path),
contents = await file.json();

const output = Bun.file(__dirname +"/index.html");
const outWriter = output.writer();
outWriter.write( html_head );
for(const name in  contents)  outWriter.write(create_card( name , contents[name] ))
outWriter.write( html_footer );
outWriter.end();

await Bun.write(__dirname+"/../www/index.html", output );