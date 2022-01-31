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

const int ROBOT_MAX_SIZE = 16 * 1024;
const int PAGE_MAX_SIZE = 2 * 1024 * 1024;

int main(int argc, char *argv[]) {

    std::string newurl;
    int threadCount;
   
    // Quit if there are too many args
    if ((argc < 2) || (argc > 3)) {
        printf("ERROR: There is more/less than one argument in the command line \nArgument count: %d", argc);
        exit(-1);
    }

    if (argc == 2) {
        newurl = argv[1];
        newurl = "http://architectureandmorality.blogspot.com/";
        DecompURL newnew;
        newnew.fillURL(newurl);
        newnew.connectURL(newnew, true, '2', true, PAGE_MAX_SIZE);
    }

    else if (argc == 3) {
        threadCount = atoi(argv[1]);

        // check if thread count is valid
        if (threadCount != 1) {
            printf("ERROR: The thread count does not equal 1\n");
            exit(-1);
        }

        // attempt to open file and send error if it doesnt exist
        std::ifstream ifs(argv[2], std::ifstream::in);
        
        if (ifs.fail()) {
            printf("ERROR: File does not exit\n");
            exit(-1);
        }

        // read file til the end line by line
        while (std::getline(ifs, newurl)) {

            DecompURL newnew;
            newnew.fillURL(newurl);

            // check host uniqueness
            printf("\t  Checking host uniqueness... ");
            if (!newnew.hostCheck(newnew.host)) {
                continue;
            }

            if (!newnew.connectURL(newnew, true, '4', true, ROBOT_MAX_SIZE)) {
                continue;
            }

            newnew.connectURL(newnew, false, '2', false, PAGE_MAX_SIZE);

        }
        

    }
    

    
    

    
}

