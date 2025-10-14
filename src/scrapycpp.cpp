#include "scrapycpp.hpp"

namespace ScrapyCpp{


void showHelp(){
	fmt::print(
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
}

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

/**
 * @cleanValue
 * */
const string cleanValue(const string &value){
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

const string getDateText(){
	time_t t = time(nullptr);
    tm* now = localtime(&t);    
    return fmt::format(
		"{} de {} {}",
		now->tm_mday,
		meses[now->tm_mon],
		(now->tm_year+1900)
	);
}

	bool HtmlDom::isFormatAccept(){
		size_t i = 0;
		while( i < formato_aceptados_len ){
			if( formato_aceptados[i] == formato) return true;
			++i;
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
			cout << "Error\n";
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
			fmt::print(
				"\n \033[31mError\033[00m: El formato '\033[33m{0}\033[00m' no es válido\n"
				" * table (por defecto)\n"
				" * json\n"
				" * txt\n\n",
				formato
				);
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
			fmt::print("{0}:{1}\n",to_lowercase(name),cleanValue(value));
		}
	}
	void HtmlDom::showJsonFormat(){
		fmt::print("{{\n");
		for(const auto &[name,value] : target_value ){			 
			 fmt::print(" \"{0}\":{1}\n",to_lowercase(name),cleanValue(value));
		}
		fmt::print("}}\n");
	}
	void HtmlDom::showTableFormat(){
		int rows = 30;
		fmt::print(
			"+{0:-^{1}}+\n"
			"|\033[32m{2: ^{1}}\033[00m|\n"
			"+{3:-^{1}}+\n",
			"",rows,getDateText(),"+");

		for(const auto &[name,value] : target_value ){			
			fmt::print("| \033[32m{1:<{0}}\033[00m| \033[33m{2:>{0}}\033[00m |\n",((rows/2)-2),name,value);
			fmt::print("+{0:-^{1}}+\n","+",rows);
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
	void HtmlDom::send(const string &url){
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
		const unsigned int target_value_size = target_value.size();
		for(int unsigned i = 0; i < target_value_size; ++i ){
			const auto &name = target_value.at(i).first;
			const auto value = cleanValue(target_value.at(i).second);
			body_json += fmt::format("\"{}\":{}{}",to_lowercase(name),value,(i != (target_value_size-1) ? ",":""));
		}
		body_json +="}";

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
	}
	//--------
	const string HtmlDom::loadContentBCentral(){

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