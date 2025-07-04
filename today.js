const axios = require('axios').default;
const cheerio = require('cheerio');
const chalk = require('chalk');
const log = console.log;
const url = "https://si3.bcentral.cl/Indicadoressiete/secure/Indicadoresdiarios.aspx";
function scrapy( _url ){
	axios.get(_url).then(function( response ){

		const $ = cheerio.load( response.data ),
		date = $("#txtDate").val(),
		uf = $("#lblValor1_1").text(),
		dolar = $("#lblValor1_3").text(),
		euro = $("#lblValor1_5").text(),
		yen = $("#lblValor1_10").text(),
		tib = $("#lblValor2_0").text();
		
		log( chalk.green("Bienvenido - indicadores económicos") );
		log( chalk.blue(date) )
		log("+"+String("-").repeat(30));
		log( "|  UF    : "+chalk.yellow(uf) )
		log("|"+String("-").repeat(30));
		log( "|  DOLAR : "+chalk.yellow(dolar) )
		log("|"+String("-").repeat(30));
		log( "|  EURO  : "+chalk.yellow(euro) )
		log("|"+String("-").repeat(30));
		log( "|  YEN   : "+chalk.yellow(yen) )
		log("|"+String("-").repeat(30));
		log( "|  TIB   : "+chalk.yellow(tib) )
		log("+"+String("-").repeat(30));
		log("");
	});

}

scrapy(url);