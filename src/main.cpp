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




// valgrind --leak-check=full --show-leak-kinds=all ./build/indicadores
int main(int argc, char * argv[]){
	std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
	argh::parser cmdl(argc, argv);
	vlBcentral v_uf{.exists=false},v_dolar{.exists=false},v_euro{.exists=false};

	const string string_today_date = to_string(now->tm_mday) + "-" +to_string(now->tm_mon+1) + "-" + to_string((now->tm_year+1900));	

	/*
	map<string,vlBcentral> bcentral_values;	
	vector<string> values = {"uf","dolar","euro"};
	for(auto s_vl:values){
		if(cmdl({s_vl})){
			bcentral_values[s_vl] = vlBcentral{.s_vl=s_vl,};
			if(cmdl(s_vl,1.0f) >> bcentral_values[s_vl].f_vl){
				bcentral_values[s_vl].exists=true;
			}
		}
	}
	*/

	if(cmdl({"--uf"})){
		v_uf.s_vl = cmdl({"--uf" }).str();
		if(cmdl("--uf",1.0f) >> v_uf.f_vl){
			v_uf.exists = true;
		}	
	}		
	if(cmdl({"--dolar"})){
		v_dolar.s_vl=cmdl({"--uf" }).str();
		if(cmdl("uf",1.0f)>>v_dolar.f_vl){
			v_dolar.exists=true;
		}
	}
	if(cmdl({"--euro" })){
		v_euro.s_vl = cmdl({"--uf" }).str();
		if(cmdl("uf",1.0f) >> v_euro.f_vl){
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
	const string valueUF=ById("lblValor1_1",document,body);
	const string valueDolar=ById("lblValor1_3",document,body);
	const string valueEuro=ById("lblValor1_5",document,body);
	const string valueYen=ById("lblValor1_10",document,body);
	// free document
	if(document != NULL){lxb_html_document_destroy(document);}
    cout << "\n";
    cout << " " <<termcolor::green << now->tm_mday << " de ";    
    cout << meses[now->tm_mon] << " " << (now->tm_year + 1900) << termcolor::reset << endl;
	const string s_separate = 	" -----------------------------\n";
	const int row_green = 12;
	const int row_yellow = 12;
	// row_green+row_yellow+5;	
	if( v_uf.exists){
		if(valueUF != "ND" ){
			cout << s_separate;	
			cout <<" "<< blockLeftGreen(string("UF(").append(v_uf.s_vl+")"), row_green) ;
			int cal_uf = v_uf.f_vl*std::stof(cleanValue(valueUF));
			cout << blockLeftYellow(int_CLP(cal_uf), row_yellow);
			cout << "|\n";		
		}		
	}
	if( v_dolar.exists){
		// se podria agregar un mensaje 
		// cunado el valor es ND para sabados y domingos
		if(valueDolar != "ND"){
			cout << s_separate;	
			cout <<" "<< blockLeftGreen(string("Dolar(").append(v_dolar.s_vl+")"), row_green) ;
			int cal_dolar = v_dolar.f_vl*std::stof(cleanValue(valueDolar));
			cout << blockLeftYellow(int_CLP(cal_dolar), row_yellow);
			cout << "|\n";		
		}
	}
	if( v_euro.exists && valueEuro != "ND" ){
		cout << s_separate;	
		cout <<" "<< blockLeftGreen(string("Euro(").append(v_euro.s_vl+")"), row_green) ;		
		int cal_euro = v_euro.f_vl*std::stof(cleanValue(valueEuro));
		cout << blockLeftYellow(int_CLP(cal_euro), row_yellow);
		cout << "|\n";		
	}
	cout << s_separate;
	cout << " "<<blockLeftGreen("UF", row_green)<<blockLeftYellow(valueUF,row_yellow)<<"|\n";
	cout << " "<<blockLeftGreen("Dolar", row_green)<<blockLeftYellow(valueDolar,row_yellow)<<"|\n";
	cout << " "<<blockLeftGreen("Euro", row_green)<<blockLeftYellow(valueEuro,row_yellow)<<"|\n";
	cout << " "<<blockLeftGreen("Yen", row_green)<<blockLeftYellow(valueYen,row_yellow)<<"|\n";
	cout << s_separate << "\n";	
    return 0;
}