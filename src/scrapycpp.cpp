#include "scrapycpp.hpp"
#include "c_print.h"

namespace ScrapyCpp{

void showVersion() {
	c_print(" Version: {s:yellow}\n","0.1.0");
}

void showHelp() {
	c_print(" {s:green}\n","Indicadores Chile");
	showVersion();
	printf("\n Modo de uso:\n");
	c_print("{s:<24}: {s}\n{s:>26}{s}\n"," -f,--formato <FORMATO>","Tipo de formato de salida","","table(por defecto),json,txt,none");
	c_print("{s:<24}: {s}\n{s:>26}{s}\n"," -s,--send <URL>","Envia la información a la url","","tipo POST(por defecto)");
	c_print("{s:<24}: {s}\n"," -nc,--no-cache","Remueve el sistema de cache");
	c_print("{s:<24}: {s}\n"," -h,--help","Modo de uso");
	c_print("{s:<24}: {s}\n", " -o,--output <PATH>", "Ruta del archivo de salida" );
	c_print("{s:<24}: {s}\n", " --silent", "Modo silencioso" );
	c_print("{s:<24}: {s}\n\n", " -v,--version", "Muestra la Versión" );
}

string to_lowercase(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return tolower(c); });
    return result;
}

string to_uppercase(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return toupper(c); });
    return result;
}

/**
 * @brief Cleans and converts numeric values from Chilean format to standard format
 *
 * Handles special cases:
 * - "ND" (No Data) returns "0.0"
 * - Removes thousand separators (dots)
 * - Converts decimal comma to decimal dot
 * Example: "39.485,65" -> "39485.65"
 *
 * @param value Raw numeric string from web page
 * @return Cleaned numeric string suitable for calculations
 */
string cleanValue(const string &value) {
	// Handle empty or "ND" (No Data - weekends/holidays)
	if(value.empty() || value == "ND")
		return "0.0";

	// Build result string efficiently
	string output;
	output.reserve(value.length());

	for(char c : value) {
		if(c != '.') {  // Remove thousand separators
			output += (c == ',' ? '.' : c);  // Convert decimal comma to dot
		}
	}
	return output;
}


/**
 * @brief Gets current date formatted in Spanish
 * @return Date string in format "DD de Month YYYY"
 */
string getDateText() {
	time_t t = time(nullptr);
	struct tm* now = localtime(&t);
	char buffer[32];  // Increased buffer size for safety
	snprintf(buffer, sizeof(buffer), "%02d de %s %d",
	        now->tm_mday,
	        MONTHS_ES[now->tm_mon],
	        now->tm_year + 1900);
	return string(buffer);
}

bool HtmlDom::isFormatAccepted() {
	for (const char* accepted : ACCEPTED_FORMATS) {
		if (format == accepted)
			return true;
	}
	return false;
}



/**
 * @brief Gets element text content by HTML element ID
 * @param id HTML element ID attribute value
 * @return Text content of the element
 */
string HtmlDom::getElementById(const string &id) {
	string text = "";
	lxb_status_t local_status;
	lxb_dom_element_t    * element;
	lxb_dom_collection_t * collection;

	collection = lxb_dom_collection_make(&document->dom_document, 128);
	local_status = lxb_dom_elements_by_attr(body, collection,
	                          (const lxb_char_t *) "id", 2,
	                          (const lxb_char_t *) id.c_str(), id.length(), true);

	if (local_status != LXB_STATUS_OK || lxb_dom_collection_length(collection) == 0) {
		c_print("{s:red}: No se pudo encontrar el elemento con ID '{s:yellow}'\n", "Error", id.c_str());
		lxb_dom_collection_destroy(collection, true);
		exit(EXIT_FAILURE);
	}

	element = lxb_dom_collection_element(collection, 0);
	text = getText(element);
	// Don't destroy element - it's owned by the collection
	lxb_dom_collection_destroy(collection, true);

	return text;
}

/**
 * @brief Extracts text content from a DOM element
 * @param element Pointer to lexbor DOM element
 * @return Concatenated text content
 */
string HtmlDom::getText(lxb_dom_element_t *element) {
	string text_content = "";
	lxb_dom_node_t * node = (lxb_dom_node_t *) element;
	lxb_dom_node_t * current_node = lxb_dom_node_first_child(node);

	while(current_node != nullptr) {
		if(current_node->type == LXB_DOM_NODE_TYPE_TEXT) {
			size_t len;
			const lxb_char_t *text = lxb_dom_node_text_content(current_node, &len);
			text_content.append((const char *) text);
		}
		current_node = lxb_dom_node_next(current_node);
	}
	return text_content;
}
/**
 * @brief Parses all configured indicators from the HTML document
 */
void HtmlDom::parseIndicators() {
	for(const auto &[indicator_name, element_id] : getTargetIndicators()) {
		const string result = getElementById(element_id);
		if(result != "ND") {
			indicator_values.push_back({indicator_name, result});
		}
	}
}

/**
 * @brief Constructor - Initializes HTML parser and fetches data
 * @param format Output format (table, json, txt, none)
 * @param use_cache Whether to use cached HTML or fetch fresh data
 */
HtmlDom::HtmlDom(const string &format, const bool &use_cache)
	: format(format), use_cache(use_cache), document(nullptr), body(nullptr) {

	if(!isFormatAccepted()) {
		c_print("\n {s:red}: El formato \"{s:yellow}\" no es válido\n"
			" * table (por defecto)\n"
			" * json\n"
			" * txt\n\n", "Error", format.c_str());
		std::exit(EXIT_FAILURE);
	}

	const string html = loadContentFromBCentral();
	document = lxb_html_document_create();
	if(document == nullptr) {
		c_print("{s:red}: No se pudo crear el documento HTML\n", "Error");
		exit(EXIT_FAILURE);
	}

	status = lxb_html_document_parse(document, (const lxb_char_t *)html.c_str(), html.length());
	if(status != LXB_STATUS_OK) {
		c_print("{s:red}: Error al parsear el documento HTML\n", "Error");
		exit(EXIT_FAILURE);
	}

	body = lxb_dom_interface_element(document->body);
	if(body == nullptr) {
		c_print("{s:red}: No se pudo obtener el body del documento\n", "Error");
		exit(EXIT_FAILURE);
	}

	parseIndicators();
}

/**
 * @brief Destructor - Cleans up lexbor resources
 */
HtmlDom::~HtmlDom() {
	if(document != nullptr) {
		lxb_html_document_destroy(document);
	}
}
/**
 * @brief Shows indicators in plain text format (key:value)
 */
void HtmlDom::showTxtFormat() {
	for(const auto &[name, value] : indicator_values) {
		c_print("{s}:{s}\n", to_lowercase(name).c_str(), cleanValue(value).c_str());
	}
}

/**
 * @brief Shows indicators in JSON format
 */
void HtmlDom::showJsonFormat() {
	printf("{\n");
	for(size_t i = 0; i < indicator_values.size(); ++i) {
		const auto &[name, value] = indicator_values[i];
		c_print(" \"{s}\":{s}{s}\n",
			to_lowercase(name).c_str(),
			cleanValue(value).c_str(),
			(i == indicator_values.size() - 1 ? "" : ",")
		);
	}
	printf("}\n");
}

/**
 * @brief Shows indicators in formatted table
 */
void HtmlDom::showTableFormat() {
	c_print("+{s:-^30}+\n", "+");
	c_print("|{s:green:^30}|\n", getDateText().c_str());
	c_print("+{s:-^30}+\n", "+");
	for(const auto &[name, value] : indicator_values) {
		c_print("| {s:green:<13}|{s:yellow:>14} |\n",
			name.c_str(),
			value.c_str()
		);
		c_print("+{s:-^30}+\n", "+");
	}
}

/**
 * @brief Displays indicators according to selected format
 */
void HtmlDom::show() {
	if(format == "table") {
		showTableFormat();
	} else if(format == "json") {
		showJsonFormat();
	} else if(format == "txt") {
		showTxtFormat();
	}
}
/**
 * @brief Saves indicators to a JSON file
 * @param path Output file path
 */
void HtmlDom::save(const string &path) {
	std::ofstream output_file(path, std::ios::out | std::ios::trunc);
	if (!output_file) {
		c_print("{s:red}: No se pudo crear el archivo '{s:yellow}'\n", "Error", path.c_str());
		return;
	}

	output_file << "{\n";

	for(size_t i = 0; i < indicator_values.size(); ++i) {
		const auto &[name, value] = indicator_values[i];
		output_file << "\t\"" << name << "\":\"" << value << "\"";
		if(i < indicator_values.size() - 1) {
			output_file << ",";
		}
		output_file << "\n";
	}

	output_file << "}";
	output_file.close();

	if(!output_file.good()) {
		c_print("{s:red}: Error al escribir en el archivo\n", "Error");
	}
}
/**
 * @brief Sends indicators as JSON to a URL via POST request
 * @param url Target URL for POST request
 */
void HtmlDom::send(const string &url) {
	auto ada_url = ada::parse(url);
	if(!ada_url) {
		if(!url.empty()) {
			c_print("{s:red}: La URL \"{s:yellow}\" no es válida\n",
				"Error", url.c_str());
		}
		return;
	}

	// Build JSON body using string concatenation
	string json_body = "{";
	const size_t values_size = indicator_values.size();

	for(size_t i = 0; i < values_size; ++i) {
		const auto &[name, raw_value] = indicator_values[i];
		const string value = cleanValue(raw_value);

		// Build JSON entry using string operations (no VLA)
		json_body += "\"" + to_lowercase(name) + "\":" + value;
		if(i != values_size - 1) {
			json_body += ",";
		}
	}
	json_body += "}";

	cpr::Response response = cpr::Post(
		cpr::Url{ada_url->get_href()},
		cpr::Body{json_body},
		cpr::Header{{"Content-Type", "application/json"}}
	);

	if(response.status_code == 0) {
		c_print("{s:red}: No se pudo enviar la información a la URL -> {s:yellow}\n",
			"Error", ada_url->get_href().data());
	}
}
/**
 * @brief Loads HTML content from Banco Central, using cache if available
 * @return HTML content as string
 */
string HtmlDom::loadContentFromBCentral() {
	// Generate cache filename based on current date
	time_t t = time(nullptr);
	tm* now = localtime(&t);
	char cache_filename[32];
	snprintf(cache_filename, sizeof(cache_filename), "%02d-%02d-%04d.html",
		now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);

	fs::path home_dir = INSTALL_BIN_DIR;
	fs::path cache_dir = home_dir / ".scrapy-indicadores";
	fs::path cache_file_path = cache_dir / cache_filename;

	// Create cache directory if it doesn't exist
	if (!fs::exists(cache_dir)) {
		if(!fs::create_directories(cache_dir)) {
			c_print("{s:red}: No se pudo crear el directorio de caché\n", "Error");
		}
	}

	// Fetch fresh content if cache doesn't exist or cache is disabled
	if(!fs::exists(cache_file_path) || !use_cache) {
		auto response = cpr::Get(cpr::Url{URL_CENTRAL});
		const string html = (response.status_code == 200) ? response.text : "";

		if(!html.empty()) {
			std::ofstream output_file(cache_file_path, std::ios::out | std::ios::trunc);
			if (!output_file) {
				c_print("{s:red}: No se pudo crear el archivo de caché\n", "Error");
			} else {
				output_file << html;
				output_file.close();
			}
		}
		return html;
	}

	// Load from cache
	string html = "";
	std::ifstream input_file(cache_file_path);
	if (!input_file.is_open()) {
		c_print("{s:red}: Error al abrir el archivo de caché\n", "Error");
		return "";
	}

	string line;
	while(std::getline(input_file, line)) {
		html.append(line);
	}
	input_file.close();

	return html;
}

}  // namespace ScrapyCpp
