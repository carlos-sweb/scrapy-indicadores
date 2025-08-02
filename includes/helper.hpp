#ifndef INDICADORES_HELPER_H
#define INDICADORES_HELPER_H
#define URl_BCENTRAL "https://si3.bcentral.cl//Indicadoressiete/secure/Indicadoresdiarios.aspx"
#include <string>
#include <fmt/base.h>
#include <lexbor/html/html.h>
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>

const char* meses[] = {
	"Enero","Febrero","Marzo", 
	"Abril", "Mayo", "Junio",
	"Julio","Agosto","Septiembre",
	"Octubre","Noviembre","Diciembre"
};

void showJsonValue(const string &uf,const string &dolar,const string &euro,const string &yen,const string &golden , const string &silver , const string &copper){
	fmt::print("{{\n \033[32m\"uf\"\033[00m : \033[33m\"{}\"\033[00m\n \033[32m\"dolar\"\033[00m : \033[33m\"{}\"\033[00m\n \033[32m\"euro\"\033[00m : \033[33m\"{}\"\033[00m\n \033[32m\"yen\"\033[00m : \033[33m\"{}\"\033[00m\n \033[32m\"oro\"\033[00m : \033[33m\"{}\"\033[00m\n \033[32m\"plata\"\033[00m : \033[33m\"{}\"\033[00m\n \033[32m\"cobre\"\033[00m : \033[33m\"{}\"\033[00m\n}}\n",uf,dolar,euro,yen,golden,silver,copper);
}

const string cleanValue(const string &valueUF){
	if(valueUF == "ND")
		return "0.0";
	string output="";	
	for(int i = (valueUF.length()-1);i >= 0;i--){
		const string val = string(1,valueUF.at(i));		
		if(val!="."){output.insert(0, val == "," ? "." : val );}
	}
	return output;		
}

const string int_CLP(const int &cal){
	string output="";
	const string body = to_string(cal);
	int position_separate=0;
	for(int i = (body.size()-1); i >= 0; i--){		
		if(position_separate == 3){
			output.insert(0,".");
			position_separate=1;
		}else{
			position_separate++;
		}
		output.insert(0,string(1,body.at(i)));
	}
	return output;
}

const std::string blockBase(
	const string &color,
	const string &text,
	int rows,
	int text_indent=1
	){	
	string output = "|";
	while(text_indent > 0){		
		output.append(" ");
		text_indent--;
	}
	output+=color;
	output+=text;
	output+="\033[00m";

	rows = (rows - text.length());
	while(rows > 0){
		output.append(" ");
		rows--;
	}	
	return output;
}
const std::string blockLeftGreen(const string &text,int rows ,int text_indent=1){	
 	return blockBase("\033[32m",text,rows,text_indent);
}
const std::string blockLeftYellow(string text,int rows ,int text_indent=1){	
 	return blockBase("\033[33m",text,rows,text_indent);
}

const std::string getText(lxb_dom_element_t *element){
    std::string text_full = "";
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

const string loadContentBCentral(){	
    auto response = cpr::Get(cpr::Url{URl_BCENTRAL});    
    return response.status_code == 200 ? response.text : "";
}

string ById(string value, lxb_html_document_t * document , lxb_dom_element_t * body){
	
	lxb_status_t status;
	lxb_dom_element_t    * element;
	lxb_dom_collection_t * collection;
	collection = lxb_dom_collection_make(&document->dom_document,128);
	string name = "id";	
	status = lxb_dom_elements_by_attr(body,collection,
	                          ( const lxb_char_t *) name.c_str(),name.length(),
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

typedef struct{
	float f_vl;
	string s_vl;
	bool exists;
} vlBcentral;


#endif