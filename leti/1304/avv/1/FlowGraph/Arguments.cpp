#include "Arguments.h"



Arguments::Arguments(int bright, int countParaleImg, std::string LogFileName, int countImg)
{
	this->bright = bright;
	this->countParaleImg = countParaleImg;
	this->LogFileName = LogFileName;
	this->countImg = countImg;

	//reqImageCount = reqBrightness = -1;
	//parallelLimit = 1;
	//good = false;
}

Arguments::Arguments()
{
	this->bright = 111;
	this->countParaleImg = 3;
	this->LogFileName = "log.txt";
	this->countImg = 5;

}

void Arguments::setArg(Arguments a)
{
	this->bright = a.bright;
	this->countParaleImg = a.countParaleImg;
	this->LogFileName = a.LogFileName;
	this->countImg = a.countImg;
}

Arguments::~Arguments()
{
}
