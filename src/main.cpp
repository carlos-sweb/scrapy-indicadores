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

const char* meses[12] = {
	"Enero","Febrero","Marzo", 
	"Abril", "Mayo", "Junio",
	"Julio","Agosto","Septiembre",
	"Octubre","Noviembre","Diciembre"
};

const string to_lowercase(const string& str) {
    string result = str;
    transform(result.begin(),
    result.end(),
    result.begin(),
	[](unsigned char c) { return tolower(c); });
    return result;
}

// Convertir a mayúsculas
const string to_uppercase(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(),
	[](unsigned char c) { return toupper(c); });
    return result;
}

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

	time_t t = time(nullptr);
    tm* now = localtime(&t);
    const string html= loadContentBCentral(cache),
    string_today_date = fmt::format(
		"{} de {} {}",
		now->tm_mday,
		meses[now->tm_mon],
		(now->tm_year+1900)
	);

	
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
	

	size_t size;
	lxb_status_t status;        	 
	lxb_html_document_t  * document;
	lxb_dom_element_t    * body;		
    document=lxb_html_document_create();
	status=lxb_html_document_parse(document,(const lxb_char_t *)html.c_str(),html.length());
	if(status!=LXB_STATUS_OK){exit(EXIT_FAILURE);}
	body = lxb_dom_interface_element(document->body);
	
	//Tratando de mejorar el codigo

	const vector<pair<string,string>> target_indicadores = {
		{"UF","lblValor1_1"},
		{"Dolar","lblValor1_3"},
		{"Euro","lblValor1_5"},
		{"Yen","lblValor1_10"},
		{"Oro","lblValor2_3"},
		{"Plata","lblValor2_4"},
		{"Cobre","lblValor2_5"}
	};
	vector<pair<string,string>> target_value = {};

	for(const auto &indicador : target_indicadores){
		const string result = ById(indicador.second,document,body);
		if(result != "ND") target_value.push_back({indicador.first,result});
	}	
	// free document
	if(document != NULL){lxb_html_document_destroy(document);}

	
	if(formato == "table"){	
		int rows = 30;
		fmt::print(
			"+{0:-^{1}}+\n"
			"|\033[32m{2: ^{1}}\033[00m|\n"
			"+{3:-^{1}}+\n",
			"",rows,string_today_date,"+");

		for(const auto &[name,value] : target_value ){			
			fmt::print("| \033[32m{1:<{0}}\033[00m| \033[33m{2:>{0}}\033[00m |\n",((rows/2)-2),name,value);
			fmt::print("+{0:-^{1}}+\n","+",rows);
		}		
		

	}else if(formato == "json"){		
		fmt::print("{{\n");
		for(const auto &[name,value] : target_value ){			 
			 fmt::print(" \"{0}\":{1}\n",to_lowercase(name),cleanValue(value));
		}
		fmt::print("}}\n");
	}else if(formato == "txt"){		
		for(const auto &[name,value] : target_value ){	
			fmt::print("{0}:{1}\n",to_lowercase(name),cleanValue(value));
		}
	}		


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
		const int target_value_size = target_value.size();
		for(int i = 0; i < target_value_size; ++i ){
			const auto &name = target_value.at(i).first;
			const auto value = cleanValue(target_value.at(i).second);
			body_json += fmt::format("\"{}\":{}{}",to_lowercase(name),value,(i != (target_value_size-1) ? ",":""));
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