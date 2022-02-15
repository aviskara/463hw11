/*
* Name: Ricky Arellano
* UIN: 728001575
* Class: CSCE 463
* Section: 500
* Semister Spring 22
*/
#include "pch.h"
#include "Socket.h"

const int INITIAL_BUF_SIZE = 1024;

Socket::Socket() {
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    buf = new char[INITIAL_BUF_SIZE];
    allocatedSize = INITIAL_BUF_SIZE;
    curpos = 0;
}

Socket::~Socket() {
    //delete[] buf;
    //buf = NULL;
}


bool Socket::Write(bool _robots, std::string _host, std::string _request)
{
    printf("\t  Loading... ");
    std::string req = "";

    // generate http request
    if (_robots) {
        req = "HEAD /robots.txt HTTP/1.0 \r\nUser-agent: hwCrawler/1.2\r\nHost: " + _host + "\r\nConnection: close\r\n\r\n";
    }
    else {
         req = "GET " + _request + " HTTP/1.1 \r\nUser-agent: hwCrawler/1.2\r\nHost: " + _host + "\r\nConnection: close\r\n\r\n";
    }
    //char* tmp = req.c_str();

    size_t reqsize = req.length();
    char* sendBuf = new char[reqsize + 1];
    std::copy(req.begin(), req.end(), sendBuf);
    sendBuf[reqsize] = '\0';

    if (send(sock, sendBuf, reqsize, 0) == SOCKET_ERROR) {
        printf("failed with: %d\n", WSAGetLastError());
        delete [] sendBuf;
        return false;
    }
    delete[]  sendBuf;
    return true;
}


bool Socket::Read(int maxSize)
{
    // set max time before listen timeout
    timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    int ret;

    fd_set fd;
    

    clock_t t = clock();


    while (true)
    {
        // initialize fd
        FD_ZERO(&fd);
        // set fd to socket
        FD_SET(sock, &fd);
        // check if there is something to read
        if ((ret = select(0, &fd, 0, 0, &timeout)) > 0)
        {
            int bytes = recv(sock, buf + curpos, allocatedSize - curpos, 0);

            // if download takes longer than 10 seconds
            if ((((double)clock() - t) / CLOCKS_PER_SEC) > 10) {
                printf("failed with slow download\n");
                return false;
            }

            if (bytes < 0) {
                printf("failed with %d\n", WSAGetLastError());
                return false;
            }
            // return as all info is sent
            if (bytes == 0) {
                buf[curpos + 1] = '\0'; 
                printf("done in %.0f ms with %d bytes\n", (1000) * ((double)clock() - t) / CLOCKS_PER_SEC, curpos);

                //// resize if buffer is greater than 32kb
                //if (allocatedSize > (32 * 1024)) {
                //    char* tmp = new char[4096];
                //    allocatedSize = 4096;
                //    delete buf;
                //    buf = tmp;
                //    
                //}
                return true;
            }
            curpos += bytes;

            // return if reading more than supposed to
            if (curpos > maxSize) {
                printf("failed with exceeding max\n");
                return false;
            }

            // resize if buffer is nto large enough
            if ((allocatedSize - curpos) < (allocatedSize/2)) {
                char* tmp = new char[static_cast<unsigned __int64>(allocatedSize) * 2];
                memcpy(tmp, buf, allocatedSize);
                allocatedSize *= 2;
                delete [] buf;
                buf = tmp;
            }
        }
        else if (ret == 0) {
            printf("failed with timeout\n");
            return false;
        }
        else {
            //std::cout << "hree2";
            printf("failed with %d\n", WSAGetLastError());
            
            return false;
        }
    }
    return false;
}

bool Socket::threadWrite(bool _robots, std::string _host, std::string _request)
{
    
    std::string req = "";

    // generate http request
    if (_robots) {
        req = "HEAD /robots.txt HTTP/1.0 \r\nUser-agent: hwCrawler/1.2\r\nHost: " + _host + "\r\nConnection: close\r\n\r\n";
    }
    else {
        req = "GET " + _request + " HTTP/1.0 \r\nUser-agent: hwCrawler/1.2\r\nHost: " + _host + "\r\nConnection: close\r\n\r\n";
    }
    //char* tmp = req.c_str();

    size_t reqsize = req.length();
    char* sendBuf = new char[reqsize + 1];
    std::copy(req.begin(), req.end(), sendBuf);
    sendBuf[reqsize] = '\0';

    if (send(sock, sendBuf, reqsize, 0) == SOCKET_ERROR) {
        delete[] sendBuf;
        return false;
    }
    delete[]  sendBuf;
    return true;
}

int Socket::threadRead(int maxSize)
{
    // set max time before listen timeout
    timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    int ret;

    fd_set fd;
    int total = 0;

    clock_t t = clock();


    while (true)
    {
        // initialize fd
        FD_ZERO(&fd);
        // set fd to socket
        FD_SET(sock, &fd);
        // check if there is something to read
        if ((ret = select(0, &fd, 0, 0, &timeout)) > 0)
        {
            int bytes = recv(sock, buf + curpos, allocatedSize - curpos, 0);
            
            // if download takes longer than 10 seconds
            if ((((double)clock() - t) / CLOCKS_PER_SEC) > 10) {
                //printf("failed with slow download\n");
                return false;
            }

            if (bytes < 0) {
                //printf("failed with %d\n", WSAGetLastError());
                return false;
            }
            // return as all info is sent
            if (bytes == 0) {
                buf[curpos + 1] = '\0';
                //printf("done in %.0f ms with %d bytes\n", (1000) * ((double)clock() - t) / CLOCKS_PER_SEC, curpos);

               // delete[] buf;
                return total;
            }
            curpos += bytes;
            total += bytes;

            // return if reading more than supposed to
            if (curpos > maxSize) {
                //printf("failed with exceeding max\n");
                return false;
            }

            // resize if buffer is greater than 32kb
            if (allocatedSize > (32 * 1024)) {
                /*char* tmp = new char[4096];
                allocatedSize = 4096;
                delete[] buf;
                buf = tmp;*/
                return false;
            }

            // resize if buffer is nto large enough
            if ((allocatedSize - curpos) < (allocatedSize/2)) {
                char* tmp = new char[static_cast<unsigned __int64>(allocatedSize) * 2];
                memcpy(tmp, buf, allocatedSize);
                allocatedSize *= 2;
                delete buf;
                buf = tmp;
            }
        }
        else if (ret == 0) {
            //printf("failed with timeout\n");
            return false;
        }
        else {
            //std::cout << "hree2";
            //printf("failed with %d\n", WSAGetLastError());

            return false;
        }
    }
    return false;
}
