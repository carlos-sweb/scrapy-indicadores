#ifndef INDICADORES_HELPER_H
#define INDICADORES_HELPER_H
#define URl_BCENTRAL "https://si3.bcentral.cl//Indicadoressiete/secure/Indicadoresdiarios.aspx"
#define URL_SII_UTM_UTA "https://www.sii.cl/valores_y_fechas/utm/utm{}.htm"
#include <string>
#include <cpr/cpr.h>
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <filesystem>
#include <fstream>
namespace fs = filesystem;

using namespace fmt;
using namespace std;


const string loadContentBCentral(bool cache){
	time_t t = time(nullptr);
    tm* now = localtime(&t);    

    const string year_str = to_string((now->tm_year+1900)),
	month_str = to_string(now->tm_mon),
    day_str = to_string(now->tm_mday) ,     
    cache_filename = fmt::format("{}-{}-{}.html",day_str,month_str,year_str);

	fs::path homeScrapy = INSTALL_BIN_DIR , 
	homeScrapyCache = homeScrapy / ".scrapy-indicadores" , 
	cache_today = homeScrapyCache / cache_filename;

	// ------------------------------------------
	if (!fs::exists(homeScrapyCache) && !fs::is_directory(homeScrapyCache)) {        
        if(fs::create_directories(homeScrapyCache)){
        	// En este punto no se pudo crear
        }
    }
    // ------------------------------------------
    if(!fs::exists(cache_today) || cache == false ){    	
    	//std::cout << "No existe el archivo \n"; OR cache es falso
		std::ofstream salida(cache_today,std::ios::out | std::ios::trunc);
		if (!salida) {
			//std::cerr << "No se pudo crear el archivo.\n";
			//return 1;
		}
		auto response = cpr::Get(cpr::Url{URl_BCENTRAL});
		const string html = response.status_code == 200 ? response.text : "";
		// Podemos procesar en caso de error
		// Falta Trabajo
		salida << html;		
		salida.close();
		return html;
    }else{
    	string html = "";
    	std::ifstream inFile;
    	inFile.open(cache_today); 
		if (!inFile.is_open()) {
			std::cerr << "Error opening file!" << std::endl;
			//return 1; // Indicate an error
		}
		string line;
		while(std::getline(inFile,line)){			
			html.append(line);
		}		
		inFile.close();
		return html;
    }
    
}




typedef struct{
	float f_vl;
	string s_vl;
	bool exists;
} vlBcentral;

#endif