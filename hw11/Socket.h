/*
* Name: Ricky Arellano
* UIN: 728001575
* Class: CSCE 463
* Section: 500
* Semister Spring 22
*/

#pragma once
#include "pch.h"
#include "DecompURL.h"

class Socket {
public:
	SOCKET sock;
	char* buf;
	int allocatedSize;
	int curpos;
	size_t size;

	Socket();

	bool Write(DecompURL _url);
	bool Read(void);
};