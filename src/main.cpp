#include "scrapycpp.hpp"
#include "argh.h"
#include <memory>

using namespace argh;

/**
 * @brief Main entry point for the Chilean economic indicators scraper
 *
 * Command-line tool to fetch and display economic indicators from Banco Central de Chile
 *
 * Usage:
 *   -h, --help              Show help message
 *   -v, --version           Show version
 *   -f, --format <FORMAT>   Output format (table, json, txt, none)
 *   -nc, --no-cache         Disable caching
 *   -s, --send <URL>        Send data via POST to URL
 *   -o, --output <PATH>     Save output to file
 *   --silent                Silent mode (no console output)
 */
int main(int, char * argv[]) {
	parser cmdl(argv);

	// Handle help and version flags
	if(cmdl[{"-h", "--help"}]) {
		ScrapyCpp::showHelp();
		return 0;
	}
	if(cmdl[{"-v", "--version"}]) {
		ScrapyCpp::showVersion();
		return 0;
	}

	// Create scraper instance with RAII
	auto scraper = std::make_unique<ScrapyCpp::HtmlDom>(
		cmdl({"-f", "--format"}, "table").str(),
		cmdl[{"-nc", "--no-cache"}]
	);

	// Process commands in order
	if(cmdl({"-s", "--send"})) {
		scraper->send(cmdl({"-s", "--send"}).str());
	}
	if(cmdl({"-o", "--output"})) {
		scraper->save(cmdl({"-o", "--output"}, "").str());
	}
	if(!cmdl[{"--silent"}]) {
		scraper->show();
	}

	return 0;
}
