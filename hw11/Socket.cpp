/*
* Name: Ricky Arellano
* UIN: 728001575
* Class: CSCE 463
* Section: 500
* Semister Spring 22
*/
#include "pch.h"
#include "Socket.h"

const int INITIAL_BUF_SIZE = 4096;

Socket::Socket() {
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    buf = new char[INITIAL_BUF_SIZE];
    allocatedSize = INITIAL_BUF_SIZE;
    curpos = 0;
    size = 4096;
}


bool Socket::Write(DecompURL _url)
{
    printf("\t  Loading... ");

    // generate http request
    std::string req = "GET " + _url.request + " HTTP/1.0 \r\nUser-agent: hwCrawler/1.1\r\nHost: " + _url.host + "\r\nConnection: close\r\n\r\n";
    //char* tmp = req.c_str();
    //std::cout << "\n" << req << "\n";

    size_t reqsize = req.length();
    char* sendBuf = new char[reqsize + 1];
    std::copy(req.begin(), req.end(), sendBuf);
    sendBuf[reqsize] = '\0';
    //printf("\n%s", sendBuf);

    if (send(sock, sendBuf, reqsize, 0) == SOCKET_ERROR) {
        printf("failed with: %d\n", WSAGetLastError());
        delete [] sendBuf;
        return false;
    }

    delete [] sendBuf;
    return true;
}


bool Socket::Read(void)
{
    // set max time before listen timeout
    timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    int ret;

    fd_set fd;
    // initialize fd
    FD_ZERO(&fd);
    // set fd to socket
    FD_SET(sock, &fd);

    clock_t t = clock();


    while (true)
    {
        
        // check if there is something to read
        if ((ret = select(0, &fd, 0, 0, &timeout)) > 0)
        {
            int bytes = recv(sock, buf + curpos, allocatedSize - curpos, 0);

            if (bytes < 0) {
                printf("failed with %d\n", WSAGetLastError());
                exit(-1);
            }
            // return as all info is sent
            if (bytes == 0) {
                buf[curpos + 1] = '\0';
                printf("done in %.0f ms with %d bytes\n", (1000) * ((double)clock() - t) / CLOCKS_PER_SEC, curpos);
                return true;
            }
            curpos += bytes;
            if (allocatedSize - curpos < 1024) {
                char* tmp = new char[size * 2];
                memcpy(tmp, buf, size);
                size *= 2;
                delete buf;
                buf = tmp;
            }
        }
        else if (ret == 0) {
            printf("failed with timeout\n");
            exit(-1);
        }
        else {
            printf("failed with %d\n", WSAGetLastError());
            exit(-1);
        }
    }
    return false;
}
