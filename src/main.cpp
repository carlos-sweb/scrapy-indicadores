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
namespace fs = std::filesystem;

#define URl_BCENTRAL "https://si3.bcentral.cl//Indicadoressiete/secure/Indicadoresdiarios.aspx"


string int_CLP(int cal){
	string output="";
	string body = to_string(cal);
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

std::string blockBase(
	string color,
	string text,
	int rows ,
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
std::string blockLeftGreen(string text,int rows ,int text_indent=1){	
 	return blockBase("\033[32m",text,rows,text_indent);
}
std::string blockLeftYellow(string text,int rows ,int text_indent=1){	
 	return blockBase("\033[33m",text,rows,text_indent);
}

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
    auto url_bcentral = cpr::Url{URl_BCENTRAL};
    auto response = cpr::Get(url_bcentral);
    if(response.status_code == 200){ html += response.text;}
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

	auto cmdl = argh::parser(argc, argv);
	float f_uf;
	string s_uf;
	bool f_uf_exists = false;

	if( cmdl({"--uf" }) ){		
		s_uf = cmdl({"--uf" }).str();
		cmdl("uf",1.0f) >> f_uf;
		f_uf_exists = true;		
	}
	
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
	// search value from ID
	string valueUF = ById("lblValor1_1",document,body);
	string valueDolar = ById("lblValor1_3",document,body);
	string valueEuro = ById("lblValor1_5",document,body);
	string valueYen = ById("lblValor1_10",document,body);
	// free document
	if(document != NULL){lxb_html_document_destroy( document );}

    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    cout << "\n";
    cout << " " <<termcolor::green << now->tm_mday << " de ";
    const char* meses[] = {
    	"enero","febrero","marzo", 
    	"abril", "mayo", "junio",
		"julio","agosto","septiembre",
		"octubre","noviembre","diciembre"
	};
    cout << meses[now->tm_mon] << " " << (now->tm_year + 1900) << termcolor::reset << std::endl;
    
	
	const int row_green = 9;
	const int row_yellow = 12;
	// row_green+row_yellow+5;	
	if( f_uf_exists ){
		cout << "--------------------------\n";	
		cout << blockLeftGreen(string("UF(").append(s_uf+")"), row_green) ;
		int cal_uf = f_uf*39280.45;
		cout << blockLeftYellow(int_CLP(cal_uf), row_yellow) ;
		cout << "|\n";		
	}
	cout << "--------------------------\n";	
	cout << blockLeftGreen("UF", row_green) << blockLeftYellow(valueUF,row_yellow)  << "|\n";
	cout << blockLeftGreen("Dolar", row_green) << blockLeftYellow(valueDolar,row_yellow) <<"|\n";
	cout << blockLeftGreen("Euro", row_green) << blockLeftYellow(valueEuro,row_yellow) <<"|\n";
	cout << blockLeftGreen("Yen", row_green) << blockLeftYellow(valueYen,row_yellow) <<"|\n";
	cout << "--------------------------\n\n";	
//LEXBOR PARSER    
    return 0;
}