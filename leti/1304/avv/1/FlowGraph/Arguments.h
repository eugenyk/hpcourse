#pragma once
//#include <string>
#include <sstream>
#include <iostream>
class Arguments
{
public:
	int bright;
	int countParaleImg;
	int countImg;
	std::string LogFileName;
	void setArg(Arguments a);
	Arguments(int bright, int countParaleImg, std::string LogFileName, int countImg);
	Arguments();

	~Arguments();
};

