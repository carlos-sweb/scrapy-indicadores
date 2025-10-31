#include "scrapycpp.hpp"
#include "argh.h"
#include <memory>
using namespace argh;
// cmake -DARGS="" -S . -B build -G Ninja && ninja -C build run
// valgrind --leak-check=full --show-leak-kinds=all ./build/indicadores
int main(int, char * argv[]){
	parser cmdl(argv);
	if(cmdl[{"-h","--help"}]){ ScrapyCpp::showHelp();return 0; }
	if(cmdl[{"-v","--version"}]){ ScrapyCpp::showVersion();return 0; }
	auto myScrapy = std::make_unique<ScrapyCpp::HtmlDom>(
		cmdl({"-f","--format"},"table").str(),
		cmdl[{"-nc","--no-cache"}]
	);
	if( cmdl({"-s","--send"}) ) myScrapy->send(cmdl({"-s","--send"}).str());
	if( cmdl({"-o","--output"}) ) myScrapy->save(cmdl({"-o","--output"},"").str());
	if(!cmdl[{"--silent"}]) myScrapy->show();
    return 0;
}
