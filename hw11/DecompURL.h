/*
* Name: Ricky Arellano
* UIN: 728001575
* Class: CSCE 463
* Section: 500
* Semister Spring 22
*/

#pragma once
#include "pch.h"
#include "Socket.h"

class DecompURL
{
public:
	std::string URL;
	std::string host;
	std::string port;
	std::string path;
	std::string query;
	std::string request;
	std::string hostip;

	Socket sock;
	Socket sock2;

	struct hostent* _remote;
	struct sockaddr_in _server;

	DecompURL();
	~DecompURL();

	int fillURL(std::string _url);
	bool connectURL(DecompURL _url, bool printHeader, char statusChar, bool _printText, int maxSize, std::unordered_set<std::string> &_map);
	bool hostCheck(std::string _url, std::unordered_set<std::string> &_map);

	bool fillThreadURL(LPVOID _param, std::string _url);
	bool connectThreadURL(LPVOID _param, DecompURL _url, bool printHeader, char statusChar, bool _printText, int maxSize, std::unordered_set<std::string>& _map);

	bool breakDownURL(std::string _url);
	bool parseDNS();
	bool uniqueCheck(std::string value, std::unordered_set<std::string>& _map);
	int connectHost(bool _isRobots, char statusChar, int _maxSize);
	



};

