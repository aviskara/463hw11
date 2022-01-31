/*
* Name: Ricky Arellano
* UIN: 728001575
* Class: CSCE 463
* Section: 500
* Semister Spring 22
*/

#include "pch.h"
#include "DecompURL.h"
#include "Socket.h"

DecompURL::DecompURL()
{
    URL = "";
    host = "";
    port = "";
    path = "";
    query = "";
    request = "";
}

DecompURL::~DecompURL()
{
}

int DecompURL::fillURL(std::string _url) 
{
    printf("URL: %s\n", _url.c_str());
    printf("\t  Parsing URL...  ");
    port = "";
    char scheme[] = "http:";
    char* url = (char*)_url.c_str();
    this->URL = url;
    char* cursor;

    // Check if the scheme is http and cut off the http://
    if (strstr(url, scheme) == NULL) {
        printf("failed with invalid scheme\n");
        return -1;
    }
    url = url + 7;

    // Find the fragment and trunc by replacing # with null
    cursor = strchr(url, '#');
    if (cursor != NULL) {
        *cursor = '\0';
    }

    // Check if the request is within the valid length
    cursor = url;
    cursor = strchr(cursor, '/');
    if (cursor != NULL) {
        int reqlen = (strlen(url) - (cursor - url));
        if (reqlen > MAX_REQUEST_LEN) {
            printf("request must be shorter than %d\n", MAX_REQUEST_LEN);
            return -2;
        }
    }


    // Search if the query and set the variable
    bool isQuery = false;
    cursor = url;
    cursor = strchr(url, '?');
    size_t querysize = 0;
    char charquery[128] = { '\0' };
    if (cursor != NULL) {
        isQuery = true;
        querysize = (strlen(url) - (cursor - url));
        memcpy(charquery, cursor, querysize + 1);
        *cursor = '\0';
    }
    cursor = charquery + querysize;
    *cursor = '\0';

    // Extract the full path
    cursor = url;
    char charpath[256] = { '\0' };
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

    // get port
    char charport[6] = { '\0' };
    char defport[] = "80";
    cursor = url;
    cursor = strchr(url, ':');
    size_t portsize = 0;

    if (cursor != NULL) {
        portsize = (strlen(url) - (cursor - url)) - 1;
        memcpy(charport, cursor + 1, portsize);
        *cursor = '\0';

        //display error if there is only : or port is 0
        if (portsize == 0) {
            printf("failed with invalid port\n");
            return -3;
        }
        else if (charport[0] == '0') {
            printf("failed with invalid port\n");
            return -4;
        }
    }
    // if not specified default to port 80
    else {
        portsize = 2;
        memcpy(charport, defport, 3);
    }

    cursor = charport + portsize;
    *cursor = '\0';

    // asssign the respective values
    host = url;
    port = charport;
    path = charpath;
    query = charquery;
    request = path + query;

    std::cout << "host " << host;
    std::cout << ", port " << port;
    std::cout << ", request " << request << "\n";

    return 1;
}

bool DecompURL::connectURL(DecompURL _url, bool _robots, char statusChar, bool _printText, int _maxSize)
{
    if (_printText) {
        printf("\t  Doing DNS... ");
    }
    clock_t t = clock();
    WSADATA wsaData;

    WORD wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        printf("WSAStartop error %d\n", WSAGetLastError());
        WSACleanup();
        return false;
    }

    Socket newsock;
    if (newsock.sock == INVALID_SOCKET) {
        printf("socket() generated error %d\n", WSAGetLastError());
        WSACleanup();
        return false;
    }

    struct hostent* remote;

    struct sockaddr_in server;

    DWORD IP = inet_addr(_url.host.c_str());
    if (IP == INADDR_NONE)
    {
        if ((remote = gethostbyname(_url.host.c_str())) == NULL)
        {
            printf("failed with %d\n", WSAGetLastError());
            return false;
        }
        else
        {
            memcpy((char*)&(server.sin_addr), remote->h_addr, remote->h_length);
        }
    }
    else
    {
        server.sin_addr.S_un.S_addr = IP;
    }
    if (_printText) {
        printf("done in %.0f ms, found %s\n", (1000) * ((double)clock() - t) / CLOCKS_PER_SEC, inet_ntoa(server.sin_addr));
    }

    // check for IP unqueness
    if (_printText) {
        printf("\t  Chekcing IP uniqueness.. ");
        std::string _ip = inet_ntoa(server.sin_addr);
        int prevSize = uniqueIP.size();
        uniqueIP.insert(_ip);

        if (!(uniqueIP.size() > prevSize)) {
            printf("failed\n");
            return false;
        }
        printf("passed\n");
    }

    if (!_robots) {
        printf("\t* Connecting on page... ");
    }
    else {
        printf("\t  Connecting on robots... ");
    }
    t = clock();

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(_url.port.c_str()));

    if (connect(newsock.sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
        printf("failed with: %d\n", WSAGetLastError());
        return false;
    }

    printf("done in %.0f ms\n", ((double)clock() - t) / CLOCKS_PER_SEC);

    
    // send and recv http request
    if (newsock.Write(_url, _robots)) 
    {
        if (newsock.Read(_maxSize)) 
        {   
            char* response = newsock.buf;
            closesocket(newsock.sock);

            char validResponse[] = "HTTP";
            char headerend[] = "\r\n\r\n";

            // get to the status and verify
            printf("\t  Verifying header... ");

            // check if response is HTTP reply
            if (memcmp(validResponse, response, 4) != 0) {
                printf("failed with non-HTTP header\n");
                return false;
            }

            char* cursor = response + 9;
            char status[4] = { '\0' };
            memcpy(status, cursor, 4);
            status[3] = '\0';
            printf("status code %s \n", status);

            // If status code is 2XX parse for links
            if (!(status[0] == statusChar)) {
                return false;
            }
            if(!_robots){
                printf("\t+ Parsing page... ");
                clock_t t = clock();

                HTMLParserBase* parser = new HTMLParserBase;

                int nLinks;
                char* linkBuffer = parser->Parse(response, (int)strlen(response), const_cast<char*>(_url.URL.c_str()), (int)strlen(_url.URL.c_str()), &nLinks);

                if (nLinks < 0)
                    nLinks = 0;
                printf("done in %.0f ms with  %d links\n", (1000) * ((double)clock() - t) / CLOCKS_PER_SEC, nLinks);
            }
            
            // extract header from response
            
            cursor = strstr(response, headerend);
            if (cursor)
                *cursor = '\0';
            
            /*
            if (!_robots) {
                printf("\n------------------------------------------\n%s\n\n", response);
            }
            */
        }
    }
    closesocket(newsock.sock);

    WSACleanup();
    return true;
}

bool DecompURL::hostCheck(std::string _url)
{
    int prevSize = uniqueURL.size();
    uniqueURL.insert(_url);

    // check to see if host was unique
    if (uniqueURL.size() > prevSize) {
        printf("passed\n");
        return true;
    }
    else {
        printf("failed\n");
        return false;
    }
}

