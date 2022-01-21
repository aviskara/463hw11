#pragma once

#include "pch.h"

class DecompURL
{
public:
	std::string URL;
	std::string host;
	std::string port;
	std::string request;

	DecompURL(std::string _url);
	~DecompURL();

	std::string PrintURL();

};

