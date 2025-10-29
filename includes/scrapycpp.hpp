#ifndef SCRAPYCPP_H
#define SCRAPYCPP_H

#include <string>
#include <vector>
#include <ctime>

#include <ada.h>
#include <cpr/cpr.h>

#include <lexbor/html/html.h>
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>

namespace ScrapyCpp{

using namespace std;
namespace fs = std::filesystem;	

static constexpr const char *url_central = "https://si3.bcentral.cl/Indicadoressiete/secure/Indicadoresdiarios.aspx";
static constexpr const char *url_sii_utm_uta = "https://www.sii.cl/valores_y_fechas/utm/utm{}.htm";
static constexpr const char* meses[12] = {"Enero","Febrero","Marzo", "Abril", "Mayo", "Junio","Julio","Agosto","Septiembre","Octubre","Noviembre","Diciembre"};
static constexpr const char* formato_aceptados[4] = {"table","json","txt","none"};

const vector<pair<string,string>> target_indicadores = {
	{"UF","lblValor1_1"},
	{"Dolar","lblValor1_3"},
	{"Euro","lblValor1_5"},
	{"Yen","lblValor1_10"},
	{"Oro","lblValor2_3"},
	{"Plata","lblValor2_4"},
	{"Cobre","lblValor2_5"}
};

void showHelp();
void showVersion();

inline const string to_lowercase(const string& str);
// Convertir a mayúsculas
inline const string to_uppercase(const string& str);

/**
 * @cleanValue
 * */
inline const string cleanValue(const string &value);

inline const char* getDateText();

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
