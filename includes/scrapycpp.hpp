#ifndef SCRAPYCPP_H
#define SCRAPYCPP_H

#include <string>
#include <vector>
#include <ctime>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <iostream>

#include <ada.h>
#include <cpr/cpr.h>

#include <lexbor/html/html.h>
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>

namespace ScrapyCpp{

using namespace std;
namespace fs = std::filesystem;	

static constexpr const char *URL_CENTRAL = "https://si3.bcentral.cl/Indicadoressiete/secure/Indicadoresdiarios.aspx";
static constexpr const char *URL_SII_UTM_UTA = "https://www.sii.cl/valores_y_fechas/utm/utm{}.htm";
static constexpr const char* MONTHS_ES[12] = {"Enero","Febrero","Marzo", "Abril", "Mayo", "Junio","Julio","Agosto","Septiembre","Octubre","Noviembre","Diciembre"};
static constexpr const char* ACCEPTED_FORMATS[4] = {"table","json","txt","none"};

/**
 * @brief Returns target indicators using Meyers Singleton pattern
 * This avoids static initialization order fiasco and ensures proper destruction order
 * @return Const reference to vector of indicator name/id pairs
 */
inline const vector<pair<string,string>>& getTargetIndicators() {
	static const vector<pair<string,string>> indicators = {
		{"UF","lblValor1_1"},
		{"Dolar","lblValor1_3"},
		{"Euro","lblValor1_5"},
		{"Yen","lblValor1_10"},
		{"Oro","lblValor2_3"},
		{"Plata","lblValor2_4"},
		{"Cobre","lblValor2_5"}
	};
	return indicators;
}

void showHelp();
void showVersion();
string to_lowercase(const string& str);
string to_uppercase(const string& str);

/**
 * @brief Cleans and converts the value from HTML to a valid numeric string
 * Removes thousand separators and converts decimal comma to dot
 * @param value Raw value from HTML
 * @return Cleaned numeric string
 */
string cleanValue(const string &value);

string getDateText();

struct HtmlDom {
	bool use_cache;
	string format;
	vector<pair<string,string>> indicator_values = {};
	lxb_status_t status;
	lxb_html_document_t  * document;
	lxb_dom_element_t    * body;

	bool isFormatAccepted();
	string getElementById(const string &id);
	string getText(lxb_dom_element_t *element);
	void parseIndicators();

	HtmlDom(const string &format, const bool &use_cache);
	~HtmlDom();

	void showTxtFormat();
	void showJsonFormat();
	void showTableFormat();
	void show();
	void send(const string &url);
	void save(const string &path);
	string loadContentFromBCentral();
};

}
#endif // SCRAPYCPP_H
