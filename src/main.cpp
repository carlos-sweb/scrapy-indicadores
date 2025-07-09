#include <strings.h>
#include <iostream>
#include <memory.h>
using namespace std;
#include <cpr/cpr.h>
#include <argh.h>
#include <ada.h>
/*
#include <termcolor/termcolor.hpp>
*/
#include <lexbor/html/html.h>
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>

std::string getText(lxb_dom_element_t *element){
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



string loadContentBCentral(){
	string html="";
	auto url = ada::parse("https://si3.bcentral.cl/");
    url->set_protocol("https");
    url->set_pathname("/Indicadoressiete/secure/Indicadoresdiarios.aspx");
    if(url){    	
        auto url_bcentral = cpr::Url{ url->get_href() };
        auto response = cpr::Get( url_bcentral );
        if(response.status_code == 200){
        	html += response.text;
        }
    }
    return html;
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

// valgrind --leak-check=full --show-leak-kinds=all ./lexbor-cpp
int main(int argc, char * argv[]){
	//auto cmdl = argh::parser(argc, argv);    
           
	string html = loadContentBCentral();
	//LEXBOR PARSER
	size_t size;
	lxb_status_t status;        	 
	lxb_html_document_t  * document;
	lxb_dom_element_t    * body;
	
	
    document = lxb_html_document_create();
	status = lxb_html_document_parse(document,( const lxb_char_t *) html.c_str(),html.length());
	if (status != LXB_STATUS_OK) {exit(EXIT_FAILURE);}
	body = lxb_dom_interface_element(document->body);


	string valueUF = ById(
		"lblValor1_1",
		document,
		body);
	string valueDolar = ById(
		"lblValor1_3",
		document,
		body);
	string valueEuro = ById(
		"lblValor1_5",
		document,
		body);

	string valueYen = ById(
		"lblValor1_10",
		document,
		body);


	cout << " UF    => " << valueUF << "\n";
	cout << " Dolar => " << valueDolar << "\n";
	cout << " Euro  => " << valueEuro << "\n";
	cout << " Yen   => " << valueYen << "\n";
	
	// buscando UF


	if(document != NULL){
		lxb_html_document_destroy( document );
	}        
//LEXBOR PARSER
    
    return 0;
}