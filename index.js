const request = require('request');
const cheerio = require('cheerio');

const url = "https://si3.bcentral.cl/Indicadoressiete/secure/Indicadoresdiarios.aspx";

request(url,function(error,response,html){
	if( !error &&  response.statusCode == 200 ){
		 
		 const $ = cheerio.load(html);
		 
		 const uf = $("#lblValor1_1").text();
		 const dolar = $("#lblValor1_3").text();
		 const euro = $("#lblValor1_5").text();
		 const yen = $("#lblValor1_10").text();
		 console.log('\x1b[33m%s\x1b[0m', "+++++++++++++++++++++++++++++++");
		 console.log('\x1b[32m%s\x1b\x1b[31m%s','UF      :','  '+uf);
		 console.log('\x1b[32m%s\x1b\x1b[31m%s','DOLAR   :','  '+dolar);
		 console.log('\x1b[32m%s\x1b\x1b[31m%s','EURO    :','  '+euro);
		 console.log('\x1b[32m%s\x1b\x1b[31m%s','YEN     :','  '+yen);
		 console.log('\x1b[33m%s\x1b[0m', "+++++++++++++++++++++++++++++++");

	};
});