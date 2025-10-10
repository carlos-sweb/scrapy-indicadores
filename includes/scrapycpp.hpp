#ifndef SCRAPYCPP_H
#define SCRAPYCPP_H

#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ranges.h> 

#include <lexbor/html/html.h>
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>


using namespace std;

namespace ScrapyCpp{



void showHelp(){

}

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

struct HtmlDom {
	const string formato;
	vector<pair<string,string>> target_value = {};
	const vector<pair<string,string>> target_indicadores = {
	{"UF","lblValor1_1"},
	{"Dolar","lblValor1_3"},
	{"Euro","lblValor1_5"},
	{"Yen","lblValor1_10"},
	{"Oro","lblValor2_3"},
	{"Plata","lblValor2_4"},
	{"Cobre","lblValor2_5"}
	};
	size_t size;
	lxb_status_t status;
	lxb_html_document_t  * document;
	lxb_dom_element_t    * body;


	const string ById( const string &value){
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
			string text =  getText(element);
			lxb_dom_element_destroy(element);
			lxb_dom_collection_destroy(collection,true);				
		}
		return getText(element);
	}

	const string getText(lxb_dom_element_t *element){
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
	void start(){
		for(const auto &indicador : target_indicadores){
			const string result = ById(indicador.second);
			if(result != "ND") target_value.push_back({indicador.first,result});
		}	
	}
	HtmlDom(const string &html,const string &formato):formato(formato){
		document=lxb_html_document_create();
		status=lxb_html_document_parse(document,(const lxb_char_t *)html.c_str(),html.length());
		if(status!=LXB_STATUS_OK){exit(EXIT_FAILURE);}
		body = lxb_dom_interface_element(document->body);
	}
	~HtmlDom(){
		if(document != NULL){lxb_html_document_destroy(document);}
		fmt::print("{}\n","Deleted");
	}
	void showTxtFormat(){
		for(const auto &[name,value] : target_value ){	
			fmt::print("{0}:{1}\n",to_lowercase(name),cleanValue(value));
		}
	}
	void showJsonFormat(){
		fmt::print("{{\n");
		for(const auto &[name,value] : target_value ){			 
			 fmt::print(" \"{0}\":{1}\n",to_lowercase(name),cleanValue(value));
		}
		fmt::print("}}\n");
	}
	void showTableFormat(){
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
	void show(){
		if(formato == "table"){			
			showTableFormat();
		}else if(formato == "json"){		
			showJsonFormat();
		}else if(formato == "txt"){	
			showTxtFormat();		
		}		
	}
};



}


#endif // SCRAPYCPP_H