#include "scrapycpp.hpp"
#include "c_print.h"

namespace ScrapyCpp{

void showVersion(){ c_print(" Version: {s:yellow}\n","0.1.0");}

void showHelp(){	
	c_print(" {s:green}\n","Indicadores Chile");
	showVersion();
	printf("\n Modo de uso:\n");
	c_print("{s:<24}: {s}\n{s:>26}{s}\n"," -f,--formato <FORMATO>","Tipo de formato de salida","","table(por defecto),json,txt,none");
	c_print("{s:<24}: {s}\n{s:>26}{s}\n"," -s,--send <URL>","Envia la información a la url","","tipo POST(por defecto)");
	c_print("{s:<24}: {s}\n"," -nc,--no-cache","Remueve el sistema de cache");
	c_print("{s:<24}: {s}\n"," -h,--help","Modo de uso");
	c_print("{s:<24}: {s}\n", " -o,--output <PATH>", "Ruta del archivo de salida" );
	c_print("{s:<24}: {s}\n", " --silent", "Modo silenciso" );
	c_print("{s:<24}: {s}\n\n", " -v,--version", "Muestra la Versión" );
}

inline const string to_lowercase(const string& str) {
    string result = str;
    transform(result.begin(),
    result.end(),
    result.begin(),
	[](unsigned char c) { return tolower(c); });
    return result;
}

// Convertir a mayúsculas
inline const string to_uppercase(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(),
	[](unsigned char c) { return toupper(c); });
    return result;
}

/**
 * @cleanValue
 * */
inline const string cleanValue(const string &value){
	// en caso que el valor capturado de la pagina web
	// del banco central retorne ND
	// Esto ocurre cuando no hay valor , fines de semana y feriados
	if(value == "ND")
		return "0.0";
	// En este punto , obtenemos una reprecentación de un numero 
	// Que luego en formato no sirve para hacer calculos matematicos, 
	// ya que los separadores de miles son puntos y los decimales son comas
	// lo cual no sirve para hacer calculos matematicos con c++ y el compilador
	// Ejemplo : UF 39.485,65 , lo que me sirve es 39485.65 ( numero flotante)
	string output="";
	for(int i = (value.length()-1);i>=0;i--){
		const string val = string(1,value.at(i));
		if(val!="."){output.insert(0, val == "," ? "." : val );}
	}
	return output;		
}


	inline const char* getDateText() {
	    time_t t = time(NULL);
	    struct tm* now = localtime(&t);	    
	    static char buffer[22]; // Buffer estático para retornar	    
	    snprintf(buffer,sizeof(buffer), "%02d de %s %d", 
	            now->tm_mday, 
	            meses[now->tm_mon], 
	            now->tm_year + 1900);	    
	    return buffer;
	}

	bool HtmlDom::isFormatAccept() {
	    for (const char* aceptado : formato_aceptados) {
	        if (formato == aceptado)
	            return true;
	    }
	    return false;
	}



	const string HtmlDom::ById( const string &value){
	string text = "";	
	lxb_status_t status;
	lxb_dom_element_t    * element;
	lxb_dom_collection_t * collection;
	collection = lxb_dom_collection_make(&document->dom_document,128);	
	status = lxb_dom_elements_by_attr(body,collection,
	                          ( const lxb_char_t *) "id" , 2 ,
	                          ( const lxb_char_t *) value.c_str(),value.length(),true);

		if (status != LXB_STATUS_OK || lxb_dom_collection_length(collection) == 0 ) {			
			printf("Error\n");
			exit(EXIT_FAILURE);
		}else{
			element = lxb_dom_collection_element(collection,0);
			text +=  getText(element);
			lxb_dom_element_destroy(element);
			lxb_dom_collection_destroy(collection,true);				
		}
		return text;
	}

	const string HtmlDom::getText(lxb_dom_element_t *element){
    string text_full = "";
    lxb_dom_node_t * node = (lxb_dom_node_t * ) element;
    lxb_dom_node_t * current_node = lxb_dom_node_first_child( node );
    while( current_node != NULL){
        if( current_node->type == LXB_DOM_NODE_TYPE_TEXT ){
            size_t len;
            const lxb_char_t *text = lxb_dom_node_text_content(current_node,&len);
            text_full.append( (const char *) text );
        }
        current_node = lxb_dom_node_next(current_node);
    }
    return text_full;
	}
	void HtmlDom::start(){
		for(const auto &indicador : target_indicadores){
			const string result = ById(indicador.second);
			if(result != "ND") target_value.push_back({indicador.first,result});
		}	
	}

	HtmlDom::HtmlDom(const string &formato , const bool &cache):formato(formato),cache(cache){	
		if(!isFormatAccept()){
			c_print("\n {s:red}: El formato \"{s:yellow}\" no es válido\n"
				" * table (por defecto)\n"
				" * json\n"
				" * txt\n\n","Error",formato.c_str());			
			std::exit(EXIT_FAILURE);
		}
		const string html = loadContentBCentral();
		document=lxb_html_document_create();
		status=lxb_html_document_parse(document,(const lxb_char_t *)html.c_str(),html.length());
		if(status!=LXB_STATUS_OK){exit(EXIT_FAILURE);}
		body = lxb_dom_interface_element(document->body);
		start();
	}
	HtmlDom::~HtmlDom(){
		if(document != NULL){lxb_html_document_destroy(document);}		
	}
	void HtmlDom::showTxtFormat(){
		for(const auto &[name,value] : target_value ){	
			c_print("{s}:{s}\n",to_lowercase(name).c_str(),cleanValue(value).c_str());
		}
	}
	void HtmlDom::showJsonFormat(){
		// Aqui hay que buscar la manera 
		// de reconocer cuando es el ultimo elemento para no agregar la
		// coma 
		printf("{\n");
		int it = 0;
		while(it < target_value.size()){
			c_print(" \"{s}\":{s}{s}\n",
				to_lowercase(target_value.at(it).first).c_str(),
				cleanValue(target_value.at(it).second).c_str(),
				(it == (target_value.size()-1) ? "": ",")
			);			
			++it;
		}		
		printf("}\n");		
	}
	void HtmlDom::showTableFormat(){
		c_print("+{s:-^30}+\n","+");
		c_print("|{s:green:^30}|\n",getDateText());
		c_print("+{s:-^30}+\n","+");
		for(const auto &[name,value] : target_value ){			
			c_print(
				"| {s:green:<13}|{s:yellow:>14} |\n",
				name.c_str(),
				value.c_str()
				);
			c_print("+{s:-^30}+\n","+");
		}
	}
	void HtmlDom::show(){		
		if(formato == "table"){			
			showTableFormat();
		}else if(formato == "json"){		
			showJsonFormat();
		}else if(formato == "txt"){	
			showTxtFormat();		
		}
	}
	void HtmlDom::save(const string &path){

		std::ofstream salida( path ,std::ios::out | std::ios::trunc);
		if (!salida) {
			std::cerr << "No se pudo crear el archivo.\n";
			//return 1;
		}
		salida << "{\n";

		auto it = target_value.begin();		
		while(it != target_value.end()){
			salida << "	\"" << it->first << "\":\"" << it->second << "\"";
			if( ++it != target_value.end() ) salida << ",";
			salida << "\n";			
		}
		// Podemos procesar en caso de error
		// Falta Trabajo
		salida << "}";
		salida.close();


	}
	void HtmlDom::send(const string &url){
	auto ada_url = ada::parse(url); 	
	if(!ada_url ){
		if(url != ""){
			c_print(
				"{s:red} La URL \"{s:yellow}\" no es válida\n",
				"Error",
				url.c_str()
			);			
		}		
	}else{		
		string body_json  = "{";		
		const unsigned int target_value_size = target_value.size();
		for(int unsigned i = 0; i < target_value_size; ++i ){
			const auto &name = target_value.at(i).first;
			const auto value = cleanValue(target_value.at(i).second);

			// Definimos el largo del pattern max
			int buffer_json_len = name.size() +  value.size() + 8;
			char buffer_json[ buffer_json_len ];	

			snprintf(buffer_json,buffer_json_len,"\"%s\":%s%s", 
				to_lowercase(name).c_str() ,
				value.c_str() , 
				(i != (target_value_size-1) ? ",":"")
			);
			body_json += buffer_json;
		}
		body_json +="}";

		cpr::Response r = cpr::Post(
			cpr::Url{ada_url->get_href()},
			cpr::Body{body_json.c_str()},
			cpr::Header{{"Content-Type","application/json"}}
		);

		switch( r.status_code ){
			case  0:				
				c_print("{s:red} : No se pudo enviar la información a la Url -> {s:yellow}\n","Error",ada_url->get_href().data() );
			break;			
		}		
	}
	}
	//--------
	const string HtmlDom::loadContentBCentral(){

		time_t t = time(nullptr);
		tm* now = localtime(&t);
		char cache_filename[16];
		snprintf(
			cache_filename,
			sizeof(cache_filename),"%02d-%02d-%04d.html",
			now->tm_mday,now->tm_mon,
			(now->tm_year+1900)
			);		

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
			auto response = cpr::Get(cpr::Url{ url_central });
			const string html = response.status_code == 200 ? response.text : "";		
			if(!html.empty()){
		    	//std::cout << "No existe el archivo \n"; OR cache es falso
				std::ofstream salida(cache_today,std::ios::out | std::ios::trunc);
				if (!salida) {
					std::cerr << "No se pudo crear el archivo.\n";
					//return 1;
				}		
				// Podemos procesar en caso de error
				// Falta Trabajo
				salida << html;		
				salida.close();
			}
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
}
