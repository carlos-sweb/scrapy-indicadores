#include <strings.h>
#include <iostream>
#include <memory.h>

using namespace std;

#include <cpr/cpr.h>
#include <argh.h>
#include <ada.h>
#include <termcolor/termcolor.hpp>



// valgrind --leak-check=full --show-leak-kinds=all ./lexbor-cpp

int main(int argc, char * argv[]){

	//auto cmdl = argh::parser(argc, argv);
    
    auto url = ada::parse("https://si3.bcentral.cl/");
    url->set_protocol("https");
    url->set_pathname("/Indicadoressiete/secure/Indicadoresdiarios.aspx");
    //url->set_pathname("/Listado/Index/"+cmdl({"-r","--remate"}).str() );
    //url->set_search("NumPag=1");
    if (url) {
        auto url_karkal = cpr::Url{ url->get_href() };
        auto response = cpr::Get( url_karkal , cpr::Timeout{5000} );
        if(response.status_code == 200){
            cout << response.text << "\n";
        }    
    }





                
                

                
            
    
    return 0;
}