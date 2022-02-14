/*
* Name: Ricky Arellano
* UIN: 728001575
* Class: CSCE 463
* Section: 500
* Semister Spring 22
*/

#pragma once
#include "pch.h"


class Socket {
public:
	SOCKET sock;
	char* buf;
	int allocatedSize;
	int curpos;

	Socket();

	bool Write(bool _robots, std::string _host, std::string _request);
	bool Read(int maxSize);

	bool threadWrite(bool _robots, std::string _host, std::string _request);
	int threadRead(int maxSize);
};