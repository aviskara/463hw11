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

	DecompURL(std::string _url);
	~DecompURL();

	std::string PrintURL();
	void connectURL(DecompURL _url);

};

