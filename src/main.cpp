#include <cstdlib>
#include <strings.h>
#include <iostream>
#include <memory.h>
using namespace std;
#include <cpr/cpr.h>
#include <argh.h>
#include <termcolor.hpp>
#include <lexbor/html/html.h>
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>
#include <ctime>
#include <iomanip>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include "helper.hpp"
namespace fs = std::filesystem;
#include <fmt/base.h>
#include <fmt/chrono.h>


void printIndicador(
	const string valueString,
	const string name ,
	const int row_green ,
	const int row_yellow
){
	
	fmt::print(" {}{}|\n",
		blockLeftGreen(name, row_green),
		blockLeftYellow(fmt::format("{:>11}",valueString),row_yellow)
	);	

}


// valgrind --leak-check=full --show-leak-kinds=all ./build/indicadores
int main(int argc, char * argv[]){
	argh::parser cmdl(argc, argv);



	const int row_green = 14,
	row_yellow = 12;
	std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);	
	vlBcentral 
		v_uf{.exists=false},
		v_dolar{.exists=false},
		v_euro{.exists=false};
	const string s_separate = " -------------------------------\n",
	string_today_date = fmt::format(
		"\033[32m{} de {} {}\033[00m",
		now->tm_mday,
		meses[now->tm_mon],
		(now->tm_year+1900)
	);

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

	const string html = loadContentBCentral();

	/*
	const char* home_dir = std::getenv("HOME");
	 if (home_dir != nullptr) {
	 	fs::path cache_path{"/home/sweb/.scrapy-indicadores/cache/13-7-2025.html"};
	 }else{
	 	fs::path cache_path{"./.scrapy-indicadores/cache/13-7-2025.html"};
	 }

	cout << cache_path.c_str() << "\n";
	if(!fs::exists(cache_path)){
		cout << "No existe \n";
		fs::create_directories(cache_path.parent_path());
		std::ofstream ofs(cache_path);
    	ofs << html;
    	ofs.close();
	}	
	*/


	size_t size;
	lxb_status_t status;        	 
	lxb_html_document_t  * document;
	lxb_dom_element_t    * body;		
    document=lxb_html_document_create();
	status=lxb_html_document_parse(document,(const lxb_char_t *)html.c_str(),html.length());
	if(status!=LXB_STATUS_OK){exit(EXIT_FAILURE);}
	body = lxb_dom_interface_element(document->body);
	// search value from ID
	const string valueUF=ById("lblValor1_1",document,body),
	valueDolar=ById("lblValor1_3",document,body),
	valueEuro=ById("lblValor1_5",document,body),
	valueYen=ById("lblValor1_10",document,body),
	valueGold=ById("lblValor2_3",document,body),
	valueSilver=ById("lblValor2_4",document,body),
	valueCopper=ById("lblValor2_5",document,body);
	// free document
	if(document != NULL){lxb_html_document_destroy(document);}

	// Solo puedo obtener el mes en forma de numero
	//fmt::print(" {:%d de %m %Y}", fmt::localtime(t));
    fmt::print("\n");
    fmt::print(" {}\n",string_today_date);

	

	// row_green+row_yellow+5;	
	if( v_uf.exists){
		if(valueUF != "ND" ){
			int cal_uf = v_uf.f_vl*std::stof(cleanValue(valueUF));
			fmt::print("{} {}{}|\n",
				s_separate,
				blockLeftGreen(fmt::format("UF({})",v_uf.s_vl), row_green),
				blockLeftYellow(fmt::format("{:>11}",int_CLP(cal_uf)), row_yellow)
			);
		}		
	}
	if( v_dolar.exists){
		// se podria agregar un mensaje 
		// cunado el valor es ND para sabados y domingos
		if(valueDolar != "ND"){			
			int cal_dolar = v_dolar.f_vl*std::stof(cleanValue(valueDolar));
			fmt::print("{} {}{}|\n",
				s_separate,
				blockLeftGreen(fmt::format("Dolar({})",v_dolar.s_vl), row_green),
				blockLeftYellow(fmt::format("{:>11}",int_CLP(cal_dolar)), row_yellow)
			);
		}
	}
	if( v_euro.exists){
		if(valueEuro != "ND"){
			int cal_euro = v_euro.f_vl*std::stof(cleanValue(valueEuro));
			fmt::print("{} {}{}|\n",
				s_separate,
				blockLeftGreen(string("Euro(").append(v_euro.s_vl+")"), row_green),
				blockLeftYellow(fmt::format("{:>11}",int_CLP(cal_euro)), row_yellow)
			);
		}		
	}

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

    return 0;
}