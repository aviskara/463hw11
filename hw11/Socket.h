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