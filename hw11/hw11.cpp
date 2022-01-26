/*
* Name: Ricky Arellano
* UIN: 728001575
* Class: CSCE 463
* Section: 500
* Semister Spring 22
*/

#include "pch.h"
#include "DecompURL.h"
//using namespace std;

int main(int argc, char *argv[]) {
   
    // Quit if there are no or more than one argument
    if (argc != 2) {
        printf("ERROR: There is more/less than one argument in the command line \nArgument count: %d", argc);
        exit(-1);
    }
    

    std::string newurl = argv[1];
    //newurl = "http://relay.tamu.edu:465/index.html";
    DecompURL newnew = DecompURL(newurl);
    newnew.connectURL(newnew);
    

    
}

