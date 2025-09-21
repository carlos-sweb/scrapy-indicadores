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
#include <argh.h>
using namespace argh;
namespace fs = filesystem;
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ranges.h>


// Convertir a minúsculas
string to_lowercase(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return tolower(c); });
    return result;
}

// Convertir a mayúsculas
string to_uppercase(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return toupper(c); });
    return result;
}

// valgrind --leak-check=full --show-leak-kinds=all ./build/indicadores
int main(int, char * argv[]){

	
	
	parser cmdl(argv);
	string formato = "";
	const string formato_aceptados[4] = {"table","json","txt"};
	bool formato_aceptado;

	if(cmdl[{"-h","--help"}]){		
		printf(" \n Indicadores Chile.\n\n Modo de uso:\n\n");		
		printf(" -f,--formato FORMATO : Tipo de formato de salida\n");
		printf("                        table,json,txt\n");
		printf(" -h,--help            : Modo de uso\n\n");
		return 0;
	}
		
	cmdl({"-f","--formato"},"table") >> formato;
	formato_aceptado = find(begin(formato_aceptados),end(formato_aceptados),formato) != end(formato_aceptados);
		
	if(!formato_aceptado){
		printf("\n \033[31mError\033[00m: El formato '\033[33m%s\033[00m' no es válido\n",formato.c_str());
		printf(" * table (por defecto)\n");
		printf(" * json\n");
		printf(" * txt\n\n");
		return 0;
	}

	time_t t = time(nullptr);
    tm* now = localtime(&t);
    const string html= loadContentBCentral(),
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

	
	if(formato == "table"){	
		int rows = 30;
		fmt::print(
			"+{0:-^{1}}+\n"
			"|\033[32m{2: ^{1}}\033[00m|\n"
			"+{3:-^{1}}+\n",
			"",rows,string_today_date,"+");

		for(const auto &[name,value] : target_value ){			
			fmt::print("|\033[32m{1:<{0}}\033[00m| \033[33m{2:>{0}}\033[00m|\n",((rows/2)-1),name,value);
		}		
		fmt::print("+{0:-^{1}}+\n","+",rows);

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
	
	
		
	
	/*
	if( result != "ND" && formato == "table") fmt::print(
			"|\033[32m{:<14}\033[00m|\033[33m{:>14}\033[00m|\n",
			indicador.first,
			result
		);		
		if( result != "ND" && formato == "json") fmt::print(
			"{{ \"{}\":{}  }}\n",
			indicador.first,
			result
		);		
	*/
	

	// free document
	if(document != NULL){lxb_html_document_destroy(document);}
	

	
	

	// row_green+row_yellow+5;	
	/*
	if( v_uf.exists){
		if(valueUF != "ND" ){
			int cal_uf = v_uf.f_vl*stof(cleanValue(valueUF));
			fmt::print("{} {}{}|\n",
				s_separate,
				blockLeftGreen(fmt::format("UF({})",v_uf.s_vl), row_green),
				blockLeftYellow(fmt::format("{:>11}",int_CLP(cal_uf)), row_yellow)
			);
		}		
	}*/
	/*
	if( v_dolar.exists){
		// se podria agregar un mensaje 
		// cunado el valor es ND para sabados y domingos
		if(valueDolar != "ND"){			
			int cal_dolar = v_dolar.f_vl*stof(cleanValue(valueDolar));
			fmt::print("{} {}{}|\n",
				s_separate,
				blockLeftGreen(fmt::format("Dolar({})",v_dolar.s_vl), row_green),
				blockLeftYellow(fmt::format("{:>11}",int_CLP(cal_dolar)), row_yellow)
			);
		}
	}*/
	/*
	if( v_euro.exists){
		if(valueEuro != "ND"){
			int cal_euro = v_euro.f_vl*stof(cleanValue(valueEuro));
			fmt::print("{} {}{}|\n",
				s_separate,
				blockLeftGreen(string("Euro(").append(v_euro.s_vl+")"), row_green),
				blockLeftYellow(fmt::format("{:>11}",int_CLP(cal_euro)), row_yellow)
			);
		}		
	}
	*/
	/*
	if( json_f ){		
		showJsonValue(valueUF,valueDolar,valueEuro,valueYen,valueGold,valueSilver,valueCopper);
	}else{		
    	fmt::print("{}",s_separate);
		printIndicador(valueUF,"UF",row_green,row_yellow);
		printIndicador(valueDolar,"Dolar",row_green,row_yellow);
		printIndicador(valueEuro,"Euro",row_green,row_yellow);
		printIndicador(valueYen,"Yen",row_green,row_yellow);
		fmt::print("{}",s_separate);
		printIndicador(valueGold,"Oro(onza)",row_green,row_yellow);
		printIndicador(valueSilver,"Plata(onza)",row_green,row_yellow);
		printIndicador(valueCopper,"Cobre(Libra)",row_green,row_yellow);
		fmt::print("{}",s_separate);	
	}*/

    return 0;
}