/*
* Name: Ricky Arellano
* UIN: 728001575
* Class: CSCE 463
* Section: 500
* Semister Spring 22
*/

#pragma once
#include "pch.h"

class DecompURL
{
public:
	std::string URL;
	std::string host;
	std::string port;
	std::string path;
	std::string query;
	std::string request;

	std::unordered_set<std::string> uniqueURL;
	std::unordered_set<std::string> uniqueIP;

	DecompURL();
	~DecompURL();

	int fillURL(std::string _url);
	bool connectURL(DecompURL _url, bool printHeader, char statusChar, bool _printText, int maxSize); 
	bool hostCheck(std::string _url);

};

