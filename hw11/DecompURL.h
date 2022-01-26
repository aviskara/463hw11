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

	DecompURL(std::string _url);
	~DecompURL();

	std::string PrintURL();
	void connectURL(DecompURL _url);

};

