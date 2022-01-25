#include "pch.h"
#include "DecompURL.h"
#include "Socket.h"

DecompURL::DecompURL(std::string _url)
{
    printf("\t  Parsing URL...  ");
    port = "";
    char scheme[] = "http:";
    char* url = (char *)_url.c_str();
    char* cursor;
   
    // Check if the scheme is http and cut off the http://
    if (strstr(url, scheme) == NULL) {
        printf("failed with invalid scheme\n");
        exit(-1);
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
    int reqlen = (strlen(url) - (cursor - url));
    if (reqlen > MAX_REQUEST_LEN) {
        printf("request must be shorter than %d\n", MAX_REQUEST_LEN);
        exit(-1);
    }

    // Check if host is within valid length
    int hostlen = (cursor - url);
    if (hostlen > MAX_HOST_LEN) {
        printf("host must be shorter than %d\n", MAX_HOST_LEN);
        exit(-1);
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
        memcpy(charquery, cursor, querysize+1);
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
        portsize = (strlen(url) - (cursor - url))-1;
        memcpy(charport, cursor+1, portsize);
        *cursor = '\0';

        //display error if there is only : or port is 0
        if (portsize == 1) {
            printf("failed with invalid port\n");
            exit(-1);
        }
        if (charport[1] == '0') {
            printf("failed with invalid port\n");
            exit(-1);
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

}

DecompURL::~DecompURL()
{
}

std::string DecompURL::PrintURL()
{
	return std::string();
}

void DecompURL::connectURL(DecompURL _url)
{
    printf("\t  Doing DNS... ");
    clock_t t = clock();
    WSADATA wsaData;

    WORD wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        printf("WSAStartop error %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    Socket newsock;
    if (newsock.sock == INVALID_SOCKET) {
        printf("socket() generated error %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    struct hostent *remote;

    struct sockaddr_in server;

    DWORD IP = inet_addr(_url.host.c_str());
    if (IP == INADDR_NONE) 
    {
        if ((remote = gethostbyname(_url.host.c_str())) == NULL) 
        {
            printf("failed with %d\n", WSAGetLastError());
            return;
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
    printf("done in %.0f ms, found %s\n", (1000) * ((double)clock() - t) / CLOCKS_PER_SEC, inet_ntoa(server.sin_addr));
    printf("\t* Connecting on page... ");
    t = clock();
    
    server.sin_family = AF_INET;
    server.sin_port = htons(80);

    if (connect(newsock.sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
        printf("failed with: %d\n", WSAGetLastError());
        return;
    }

    printf("done in %.0f ms\n", ((double)clock() - t) / CLOCKS_PER_SEC);

    // send and recv http request
    if (newsock.Write(_url)) 
    {
        if (newsock.Read()) 
        {
            char* response = newsock.buf;
            closesocket(newsock.sock);

            // get to the status and verify
            printf("\t  Verifying header... ");
            char* cursor = response + 9;
            char status[4] = { '\0' };
            memcpy(status, cursor, 4);
            status[3] = '\0';
            printf("status code %s \n", status);

            // If status code is 2XX parse for links
            if ((atoi(status) >= 200) && (atoi(status) < 300)) {
                printf("\t+ Parsing page... ");
                clock_t t = clock();
                HTMLParserBase* parser = new HTMLParserBase;

                int nLinks;
                char* linkBuffer = parser->Parse(response, (int)strlen(response) , const_cast<char*>(_url.URL.c_str()), (int)strlen(_url.URL.c_str()), &nLinks);

                if (nLinks < 0)
                    nLinks = 0;
                printf("done in %.0f ms with  %d links\n", (1000) * ((double)clock() - t) / CLOCKS_PER_SEC, nLinks);
            }

            // extract header from response
            char headerend[] = "\r\n\r\n";
            cursor = strstr(response, headerend);
            if (cursor)
                *cursor = '\0';
            
            printf("\n------------------------------------------\n%s\n\n", response);

        }
    }

    closesocket(newsock.sock);

    WSACleanup();
}
