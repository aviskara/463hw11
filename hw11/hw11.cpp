// hw11.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "DecompURL.h"
//using namespace std;

int main(int argc, char *argv[]) {
   
    // Quit if there are no or more than one argument
    if (argc != 2) {
        printf("ERROR: There is more/less than one argument in the command line \nArgument count: %d", argc);
        exit(-1);
    }
    std::cout << "URL: " << argv[1] << "\n";

    std::string newurl = argv[1];
    newurl = "https://yahoo.com/";
    DecompURL newnew = DecompURL(newurl);
    newnew.connectURL(newnew);
    

    
}

