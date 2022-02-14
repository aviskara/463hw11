/*
* Name: Ricky Arellano
* UIN: 728001575
* Class: CSCE 463
* Section: 500
* Semister Spring 22 aaaa
*/

#include "pch.h"
#include "DecompURL.h"
//using namespace std;

const int ROBOT_MAX_SIZE = 16 * 1024;
const int PAGE_MAX_SIZE = 2 * 1024 * 1024;
int printTime = 2;

struct Parameters {
    HANDLE queueMutex;
    HANDLE counterMutex;
    HANDLE listMutex;
    HANDLE list2Mutex;
    HANDLE timerMutex;
    HANDLE timer2Mutex;
    HANDLE finished;
    HANDLE evenetQuit;

    HANDLE emptyQueueSlots;
    HANDLE fullQueueSlots;

    std::queue<std::string> sharedQueue;

    std::unordered_set<std::string> uniqueURL;
    std::unordered_set<std::string> uniqueIP;

    std::string fileName;

    // values for 2 sec interval output
    volatile int pendingQueue = 0;
    volatile int extractedURL = 0;
    volatile int uniqueHostCount = 0;
    volatile int DNSCount = 0;
    volatile int uniqueIPCount = 0;
    volatile int robotPassCount = 0;
    volatile int crawledURLCount = 0;
    volatile int linksFound = 0;
    volatile int activeThreads = 0;
    volatile int previousCrawl = 0;
    volatile size_t currentBits = 0;
    volatile size_t pasBits = 0;
    volatile size_t parseBits = 0;

    // end time counter
    volatile double extractTime = 0;
    volatile double lookupTime = 0;
    volatile double robotsTime = 0;
    volatile double crawlTime = 0;
    volatile double parseTime = 0;

    // http codes recieved 
    volatile int respCodes[5] = { 0 };

    // tamu link count.
    volatile int tamuCount = 0;
};

bool DecompURL::fillThreadURL(LPVOID _param, std::string _url)
{
    Parameters* p = (Parameters*)_param;

    //printf("URL: %s\n", _url.c_str());
    //printf("\t  Parsing URL...  ");
    port = "";
    char scheme[] = "http:";
    char* url = (char*)_url.c_str();
    this->URL = url;
    char* cursor;

    // check if past max length
    if (_url.length() > 2048) {
        return false;
    }

    // Check if the scheme is http and cut off the http://
    if (strstr(url, scheme) == NULL) {
        //printf("failed with invalid scheme\n");
        return false;
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
            //printf("request must be shorter than %d\n", MAX_REQUEST_LEN);
            return false;
        }
    }


    // Search if the query and set the variable
    bool isQuery = false;
    cursor = url;
    cursor = strchr(url, '?');
    size_t querysize = 0;
    if (cursor != NULL) {
        isQuery = true;
        querysize = (strlen(url) - (cursor - url));
        *cursor = '\0';
    }


    // Extract the full path
    cursor = url;

    cursor = strchr(cursor, '/');

    //check if path is too long
    if ((cursor != NULL) && (strlen(cursor) > 2048)) {
        return false;
    }
    char charpath[2048] = { '\0' };
    size_t pathsize = 1;

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
            //printf("failed with invalid port\n");
            return false;
        }
        else if (charport[0] == '0') {
            //printf("failed with invalid port\n");
            return false;
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
    request = path + query;


    return true;
}

bool DecompURL::connectThreadURL(LPVOID _Param, DecompURL _url, bool _robots, char statusChar, bool _printText, int _maxSize, std::unordered_set<std::string>& _map)
{
    Parameters* p = (Parameters*)_Param;
    clock_t t = clock();

    if (_printText) {
        //printf("\t  Doing DNS... ");
    }
    WSADATA wsaData;

    WORD wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        //printf("WSAStartop error %d\n", WSAGetLastError());
        WSACleanup();
        return false;
    }

    Socket newsock;
    if (newsock.sock == INVALID_SOCKET) {
        //printf("socket() generated error %d\n", WSAGetLastError());
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
            //printf("failed with %d\n", WSAGetLastError());
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
        //printf("done in %.0f ms, found %s\n", (1000) * ((double)clock() - t) / CLOCKS_PER_SEC, inet_ntoa(server.sin_addr));
    }

    if (_robots) {
        WaitForSingleObject(p->timer2Mutex, INFINITE);
        p->DNSCount += 1;
        ReleaseMutex(p->timer2Mutex);
    }
    

    // check for IP unqueness
    if (_printText) {
        //printf("\t  Chekcing IP uniqueness.. ");
        std::string _ip = inet_ntoa(server.sin_addr);

        WaitForSingleObject(p->list2Mutex, INFINITE);
        int prevSize = _map.size();
        _map.insert(_ip);
        p->uniqueIPCount += 1;
        ReleaseMutex(p->list2Mutex);

        if (!(_map.size() > prevSize)) {
            //printf("failed\n");
            return false;
        }
        //printf("passed\n");
    }

    if (!_robots) {
        //printf("\t* Connecting on page... ");
    }
    else {
        //printf("\t  Connecting on robots... ");
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(_url.port.c_str()));

    if (connect(newsock.sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
        //printf("failed with: %d\n", WSAGetLastError());
        return false;
    }

    //printf("done in %.0f ms\n", ((double)clock() - t) / CLOCKS_PER_SEC);


    // send and recv http request
    if (newsock.threadWrite(_robots, host, request))
    {
        int bytes = newsock.threadRead(_maxSize);
        //std::cout << "bytes is " << bytes;
        if (bytes > 0)
        {
            if (_robots) {
                WaitForSingleObject(p->timerMutex, INFINITE);
                p->currentBits += (bytes * 8);
                ReleaseMutex(p->timerMutex);
            }
            else {
                WaitForSingleObject(p->timerMutex, INFINITE);
                p->currentBits += (bytes * 8);
                p->parseBits += (bytes * 8);
                ReleaseMutex(p->timerMutex);
            }
            char* response = newsock.buf;
            closesocket(newsock.sock);

            char validResponse[] = "HTTP";
            char headerend[] = "\r\n\r\n";

            // get to the status and verify
            //printf("\t  Verifying header... ");

            // check if response is HTTP reply
            if (memcmp(validResponse, response, 4) != 0) {
                //printf("failed with non-HTTP header\n");
                return false;
            }

            char* cursor = response + 9;
            char status[4] = { '\0' };
            memcpy(status, cursor, 4);
            status[3] = '\0';
            //printf("status code %s \n", status);

            // If status code is 2XX parse for links
            if (!(status[0] == statusChar)) {
                return false;
            }
            if (!_robots) {
                //printf("\t+ Parsing page... ");
                if (status[0] == '2') {
                    WaitForSingleObject(p->counterMutex, INFINITE);
                    p->respCodes[0] += 1;
                    if ((_url.host.size() >= 8)) {
                        if (_url.host.substr(_url.host.size() - 8) == "tamu.edu") {
                            p->tamuCount += 1;
                        }
                    }
                    
                    ReleaseMutex(p->counterMutex);
                }
                else if (status[0] == '3') {
                    WaitForSingleObject(p->counterMutex, INFINITE);
                    p->respCodes[1] += 1;
                    ReleaseMutex(p->counterMutex);
                }
                else if (status[0] == '4') {
                    WaitForSingleObject(p->counterMutex, INFINITE);
                    p->respCodes[2] += 1;
                    ReleaseMutex(p->counterMutex);
                }
                else if (status[0] == '5') {
                    WaitForSingleObject(p->counterMutex, INFINITE);
                    p->respCodes[3] += 1;
                    ReleaseMutex(p->counterMutex);
                }
                else {
                    WaitForSingleObject(p->counterMutex, INFINITE);
                    p->respCodes[4] += 1;
                    ReleaseMutex(p->counterMutex);
                }

                HTMLParserBase* parser = new HTMLParserBase;

                int nLinks;
                char* linkBuffer = parser->Parse(response, (int)strlen(response), const_cast<char*>(_url.URL.c_str()), (int)strlen(_url.URL.c_str()), &nLinks);

                if (nLinks < 0)
                    nLinks = 0;

                WaitForSingleObject(p->timerMutex, INFINITE);
                p->linksFound += nLinks;
                ReleaseMutex(p->timerMutex);

                //printf("done in %.0f ms with  %d links\n", (1000) * ((double)clock() - t) / CLOCKS_PER_SEC, nLinks);
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
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    closesocket(newsock.sock);
    return true;
}

UINT sig_handler(LPVOID _Param) {
    Parameters* p = (Parameters*)_Param;
    
    clock_t t = clock();
    clock_t prev_t = clock();
    int currentTime = 0;
    int elapsedTime = 0;
    double megabit = 1024 * 1024;

    while (true) {
        t = clock();
        elapsedTime = (int)((t - prev_t) / CLOCKS_PER_SEC);
        if (elapsedTime >= 2) {
            currentTime += elapsedTime;
            printf("[%3d] %5d Q %6d E %7d H %6d D %6d I %5d R %5d C %5d L %4dK\n\t*** crawling %.1f pps @ %.1f Mbps\n",
                currentTime, p->activeThreads, p->pendingQueue, p->extractedURL, p->uniqueHostCount, p->DNSCount,
                p->uniqueIPCount, p->robotPassCount, p->crawledURLCount, (p->linksFound/1000),
                (((double)p->crawledURLCount-(double)p->previousCrawl)/2), ((p->currentBits - p->pasBits) / (2*megabit)));
            prev_t = t;
            //WaitForSingleObject(p->timerMutex, INFINITE);
            p->previousCrawl = p->crawledURLCount;
            p->pasBits = p->currentBits;
            //ReleaseMutex(p->timerMutex);
        }
    }
    return 0;
}

UINT connectionThread(LPVOID _Param) {

    Parameters* p = (Parameters*)_Param;
    std::string currenturl;

    WaitForSingleObject(p->counterMutex, INFINITE);
    p->activeThreads += 1;
    ReleaseMutex(p->counterMutex);

    while (true) {
        WaitForSingleObject(p->queueMutex, INFINITE);
        if(!p->sharedQueue.empty()){
            currenturl = p->sharedQueue.front();
            p->sharedQueue.pop();
        }
        else
        {
            break;
        }
        ReleaseMutex(p->queueMutex);

        DecompURL urlStruct;
        if (urlStruct.fillThreadURL(p, currenturl) < 0) {
            continue;
        }
        WaitForSingleObject(p->counterMutex, INFINITE);
        p->extractedURL += 1;
        p->pendingQueue -= 1;
        ReleaseMutex(p->counterMutex);

        //// check url uniqueness
        WaitForSingleObject(p->listMutex, INFINITE);
        if (!urlStruct.uniqueCheck(urlStruct.host, p->uniqueURL)) {
            continue;
        }
        p->uniqueHostCount += 1;
        ReleaseMutex(p->listMutex);

        if (!urlStruct.connectThreadURL(p, urlStruct, true, '4', true, ROBOT_MAX_SIZE, p->uniqueIP)) {
            continue;
        }
        WaitForSingleObject(p->counterMutex, INFINITE);
        p->robotPassCount += 1;
        ReleaseMutex(p->counterMutex);

        if (!urlStruct.connectThreadURL(p, urlStruct, false, '2', false, PAGE_MAX_SIZE, p->uniqueIP)) {
            continue;
        }
        WaitForSingleObject(p->counterMutex, INFINITE);
        p->crawledURLCount += 1;
        ReleaseMutex(p->counterMutex);



    }
    WaitForSingleObject(p->counterMutex, INFINITE);
    p->activeThreads -= 1;
    ReleaseMutex(p->counterMutex);

    return 0;
}

UINT fileReaderThread(LPVOID _Param) {

    std::string newurl;
    Parameters* p = (Parameters*)_Param;

    WaitForSingleObject(p->counterMutex, INFINITE);
    p->activeThreads += 1;
    ReleaseMutex(p->counterMutex);

    // attempt to open file andsend error if it doesnt exist
    std::ifstream ifss(p->fileName, std::ios::binary);

    if (ifss.fail()) {
        printf("ERROR: File does not exit\n");
        exit(-1);
    }

    // get file size 
    ifss.seekg(0, std::ios::end);
    int filesize = ifss.tellg();

    printf("Opened %s with size %d\n", p->fileName.c_str(), filesize);

    ifss.close();

    std::ifstream ifs(p->fileName, std::ifstream::in);

    while (std::getline(ifs, newurl)) {
        p->sharedQueue.push(newurl);
        p->pendingQueue += 1;
    }
    ifs.close();

    WaitForSingleObject(p->counterMutex, INFINITE);
    p->activeThreads -= 1;
    ReleaseMutex(p->counterMutex);

    return 0;
}

int main(int argc, char *argv[]) {

    std::string newurl;
    int threadCount;
   
    // Quit if there are too many args
   

    if (argc == 2) {
        newurl = argv[1];
        DecompURL newnew;
        newnew.fillURL(newurl);
        //newnew.connectURL(newnew, true, '2', true, PAGE_MAX_SIZE, uniqueIP);
    }

    else if (argc == 3) {

        threadCount = atoi(argv[1]);
        int maxURLTextSize = 110000;

        // check if thread count is valid
        if (threadCount < 1) {
            printf("ERROR: The thread count is not greater than 1\n");
            exit(-1);
        }

        Parameters p;
        clock_t t = clock();



        // initialize semaphores
        p.queueMutex = CreateMutex(NULL, 0, NULL);
        p.counterMutex = CreateMutex(NULL, 0, NULL);
        p.timerMutex = CreateMutex(NULL, 0, NULL);
        p.timer2Mutex = CreateMutex(NULL, 0, NULL);
        p.listMutex = CreateMutex(NULL, 0, NULL);
        p.list2Mutex = CreateMutex(NULL, 0, NULL);
        p.emptyQueueSlots = CreateSemaphore(NULL, 1, 1, NULL);
        p.fullQueueSlots = CreateSemaphore(NULL, 0, maxURLTextSize, NULL);

        // initialize mutex for parameters

        // initialize default values
        p.fileName = argv[2];
        
        // begin to populate sharedFilequeue
        HANDLE fileReader = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fileReaderThread, &p, 0, NULL);
        HANDLE printer = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)sig_handler, &p, 0, NULL);

        WaitForSingleObject(fileReader, INFINITE);
        CloseHandle(fileReader);

        double readtime = ((double)clock() - t) / CLOCKS_PER_SEC;

        //printf("done in %f sec\n", ((double)clock() - t) / CLOCKS_PER_SEC);
        t = clock();
        int threads = std::stoi(argv[1]);
        HANDLE* handles = new HANDLE[threads];

        for (int i = 0; i < threads; i++) {
            handles[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)connectionThread, &p, 0, NULL);
        }

        for (int i = 0; i < threads; i++) {
            WaitForSingleObject(handles [i], INFINITE);
            CloseHandle(handles [i]);
        }

        CloseHandle(printer);
        
        int endtime = (((double)clock() - t) / CLOCKS_PER_SEC);

        printf("\nExtracted %d URLs @ %d/s\n", p.extractedURL, p.extractedURL/endtime);
        printf("Looked up %d DNS names @ %d/s\n", p.DNSCount, p.DNSCount/endtime);
        printf("Attempted %d robots @ %d/s\n", p.uniqueIPCount, p.uniqueIPCount/endtime);
        printf("Crawled %d pages @ %d/s (%.2f MB)\n", p.crawledURLCount, p.crawledURLCount/endtime, ((double)p.parseBits/(8* 1024 * 1024)));
        printf("Parsed %d links @ %d/s\n", p.crawledURLCount, p.crawledURLCount/endtime);
        printf("HTTP codes: 2xx = %d, 3xx = %d, 4xx = %d, 5xx = %d, other = %d\n",
            p.respCodes[0], p.respCodes[1], p.respCodes[2], p.respCodes[3], p.respCodes[4]);
        printf("tamu count: %d\n", p.tamuCount);
        /*while (!p.sharedQueue.empty()) {
            std::cout << p.sharedQueue.front() << " ";
            p.sharedQueue.pop();
        }
        printf("done in %.0f ms with %d bytes\n", (1000) * ((double)clock() - t) / CLOCKS_PER_SEC);*/
        

    }
    else {
        if ((argc < 2) || (argc > 3)) {
            printf("ERROR: There is more/less than one argument in the command line \nArgument count: %d", argc);
            exit(-1);
        }
    }
    
    return 0;
    
}

