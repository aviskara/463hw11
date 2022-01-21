#include "pch.h"
#include "DecompURL.h"

DecompURL::DecompURL(std::string _url)
{
    printf("Parsing URL...  ");
    char scheme[] = "http:";
    char* url = (char *)_url.c_str();
    char* cursor;

    // Check if the scheme is http and cut off the http://
    if (strstr(url, scheme) == NULL) {
        printf("\nERROR: Invalid scheme, format must be http\n");
        exit(-1);
    }
    url = url + 7;

    // Find the fragment and trunc by replacing # with null
    cursor = strchr(url, '#');
    if (cursor != NULL) {
        *cursor = '\0';
        printf("url: %s \n", url);
    }

    // Search if the query and set the variable
    bool isQuery = false;
    cursor = url;
    cursor = strchr(url, '?');
    if (cursor != NULL) {
        isQuery = true;
        char charquery[128];
        size_t querysize = (strlen(url) - (cursor - url));
        memcpy(charquery, cursor, querysize+1);
        *cursor = '\0';
        printf("query: %s\n", charquery);
    }

    // Extract the full path
    cursor = url;
    char charpath[256];
    size_t pathsize = 1;
    cursor = strchr(cursor, '/');

    if (cursor == NULL) {
		// if there is no '/' after the ://
        pathsize = 1;
        *charpath = '/';
    }
    else {
        pathsize = (strlen(url) - (cursor - url));
        memcpy(charpath, cursor, pathsize);
        *cursor = '\0';
    }
    cursor = charpath + pathsize;
    *cursor = '\0';
    printf("path: %s\n", charpath);

    // get port
    char charport[6];
    cursor = url;
    cursor = strchr(url, ':');
    size_t portsize = 0;
    if (cursor != NULL) {
        portsize = (strlen(url) - (cursor - url));
        memcpy(charport, cursor, portsize);
        *cursor = '\0';
    }
    cursor = charport + portsize;
    *cursor = '\0';
    printf("port: %s\n", charport);

    printf("host: %s\n", url);

}

DecompURL::~DecompURL()
{
}

std::string DecompURL::PrintURL()
{
	return std::string();
}
