#ifndef SCRAPYCPP_H
#define SCRAPYCPP_H

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <cstdint>
#include <fstream>

#include <ada.h>
#include <cpr/cpr.h>
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <lexbor/html/html.h>
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>


namespace ScrapyCpp{

using namespace std;
namespace fs = std::filesystem;	

extern const char *url_central; 
extern const char *url_sii_utm_uta;
extern const char* meses[12];
extern const string formato_aceptados[4];
extern const vector<pair<string,string>> target_indicadores;

void showHelp();
	
const string to_lowercase(const string& str);
// Convertir a mayúsculas
const string to_uppercase(const string& str);

/**
 * @cleanValue
 * */
const string cleanValue(const string &value);

const string getDateText();

struct HtmlDom {    

	bool cache;
	string formato;
	vector<pair<string,string>> target_value = {};

	size_t size;
	lxb_status_t status;
	lxb_html_document_t  * document;
	lxb_dom_element_t    * body;
	bool isFormatAccept();
	const string ById( const string &value);
	const string getText(lxb_dom_element_t *element);
	void start();
	HtmlDom(const string &formato , const bool &cache);
	~HtmlDom();
	void showTxtFormat();
	void showJsonFormat();
	void showTableFormat();
	void show();
	void send(const string &url);	
	const string loadContentBCentral();
};

}
#endif // SCRAPYCPP_H