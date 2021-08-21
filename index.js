const request = require('request');
const cheerio = require('cheerio');
var cron      = require('node-cron');
var moment = require('moment');

const Database = require('better-sqlite3');
//const db = new Database('indicadores.sqlite3', { verbose: console.log });
const db = new Database('indicadores.sqlite3', {});
const url = "https://si3.bcentral.cl/Indicadoressiete/secure/Indicadoresdiarios.aspx";


cron.schedule('* * * * *', () => {

	request(url,function(error,response,html){
		if( !error &&  response.statusCode == 200 ){

			 const $ = cheerio.load(html);
			 const uf = $("#lblValor1_1").text();
			 const dolar = $("#lblValor1_3").text();
			 const euro = $("#lblValor1_5").text();
			 const yen = $("#lblValor1_10").text();

			 /*
			 console.log('\x1b[33m%s\x1b[0m', "+++++++++++++++++++++++++++++++");
			 console.log('\x1b[32m%s\x1b\x1b[31m%s','UF      :','  '+uf);
			 console.log('\x1b[32m%s\x1b\x1b[31m%s','DOLAR   :','  '+dolar);
			 console.log('\x1b[32m%s\x1b\x1b[31m%s','EURO    :','  '+euro);
			 console.log('\x1b[32m%s\x1b\x1b[31m%s','YEN     :','  '+yen);
			 console.log('\x1b[33m%s\x1b[0m', "+++++++++++++++++++++++++++++++");
			 */

			const insert = db.prepare("INSERT INTO indicadores (uf,dolar,euro,yen,date) SELECT * FROM ( SELECT  ? as vl_uf , ? as vl_dolar , ? as vl_euro ,? as vl_yen, ? as cl_date ) WHERE NOT EXISTS( SELECT * FROM indicadores WHERE date = ? )");
			const  now =  moment().format('MM-DD-YYYY');
			insert.run( uf , dolar , euro , yen , now , now  );

		};
	});

});
