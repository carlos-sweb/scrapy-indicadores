#include <cstdlib>
#include <strings.h>
#include <iostream>
#include <memory.h>
using namespace std;
#include <ctime>
#include <iomanip>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include "scrapycpp.hpp"
#include "helper.hpp"
using namespace std;
#include "argh.h"
using namespace argh;
namespace fs = filesystem;
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ranges.h> 
#include <ada.h>




// valgrind --leak-check=full --show-leak-kinds=all ./build/indicadores
int main(int, char * argv[]){
	
	parser cmdl(argv);
	string formato = "",
	url = "";	
	const string formato_aceptados[4] = {"table","json","txt","none"};
	bool formato_aceptado , cache = true;


	if(cmdl[{"-h","--help"}]){		
		
			fmt:print(
			"\n"
			" {0}\n\n"
			" Modo de uso:\n"
			"\n"			
			" -f,--formato FORMATO  : Tipo de formato de salida\n"
			"                         table(por defecto),json,txt,none\n"
			" -s,--send URL         : Envia la información a la url\n"
			"                         tipo POST(por defecto)\n"
			" -nc,--no-cache        : Remueve el sistema de cache\n"
			" -h,--help             : Modo de Uso\n"
			"\n",
			"Indicadores Chile"
		);

		return 0;		
	}

	cmdl({"-f","--formato"},"table") >> formato;	

	if(cmdl[{"-nc","--no-cache"}]){ cache = false; }	

	formato_aceptado = find(begin(formato_aceptados),end(formato_aceptados),formato) != end(formato_aceptados);
		
	if(!formato_aceptado){
		fmt::print(
			"\n \033[31mError\033[00m: El formato '\033[33m{0}\033[00m' no es válido\n"
			" * table (por defecto)\n"
			" * json\n"
			" * txt\n\n",
			formato
			);		
		return 0;
	}

   
	bool json_f = false;
	const int row_green = 14,
	row_yellow = 12;
	
	vlBcentral 
		v_uf{.exists=false},
		v_dolar{.exists=false},
		v_euro{.exists=false};
	const string s_separate = " -------------------------------\n";
	if(cmdl({"uf"})){
		v_uf.s_vl = cmdl({"uf" }).str();
		if(cmdl("uf",1.0f) >> v_uf.f_vl){
			v_uf.exists = true;
		}
	}
	if(cmdl({"dolar"})){
		v_dolar.s_vl=cmdl({"dolar" }).str();
		if(cmdl("dolar",1.0f)>>v_dolar.f_vl){
			v_dolar.exists=true;
		}
	}
	if(cmdl({"euro" })){
		v_euro.s_vl = cmdl({"euro" }).str();
		if(cmdl("euro",1.0f) >> v_euro.f_vl){
			v_euro.exists = true;
		}
	}

	const string html= loadContentBCentral(cache);	
	ScrapyCpp::HtmlDom *parse = new ScrapyCpp::HtmlDom(html,formato);	
	parse->start();
	parse->show();
		
	

	


	// SECTION DE ENVIO DE DATOS
	cmdl({"-s","--send"},"") >> url;			
	auto ada_url = ada::parse(url); 	
	if(!ada_url ){
		if(url != ""){
			fmt::print(
		"\n \033[31mError\033[00m: La URL '\033[33m{0}\033[00m' no es válida\n\n",
		url
		);
		}		
	}else{		
		string body_json  = "{";		
		const int target_value_size = parse->target_value.size();
		for(int i = 0; i < target_value_size; ++i ){
			const auto &name = parse->target_value.at(i).first;
			const auto value = ScrapyCpp::cleanValue(parse->target_value.at(i).second);
			body_json += fmt::format("\"{}\":{}{}",ScrapyCpp::to_lowercase(name),value,(i != (target_value_size-1) ? ",":""));
		}
		body_json +="}";

		//fmt::print("{}\n",body_json);

		cpr::Response r = cpr::Post(
			cpr::Url{ada_url->get_href()},
			cpr::Body{body_json.c_str()},
			cpr::Header{{"Content-Type","application/json"}}
		);

		switch( r.status_code ){
			case  0:
				fmt::print("\033[31mError\033[00m : No se pudo enviar la información a la Url -> \033[33m{}\033[00m\n",ada_url->get_href());
			break;			
		}		
	}
	// SECTION DE ENVIO DE DATOS	
    return 0;
}
