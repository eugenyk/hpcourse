#include "ParmParser.h"

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

Parameters ParmParser::parse(int argc, char * argv[])
{
	parms = Parameters();
	stringstream ss;

	for (int i = 0; i < argc; i++)
		ss << argv[i] << ' ';

	for (string parmName; ss >> parmName; )
	{
		if (parmName == "-b")
		{
			parseBrightness(ss);
			continue;
		}

		if (parmName == "-l")
		{
			parseLimit(ss);
			continue;
		}

		if (parmName == "-k")
		{
			parseCount(ss);
			continue;
		}

		if (parmName == "-f")
		{
			parseLogFilename(ss);
			continue;
		}
	}

	checkRequirements();

	return parms;
}

void ParmParser::parseBrightness(stringstream & ss)
{
	ss >> parms.reqBrightness;
	if (parms.reqBrightness < 0 || parms.reqBrightness > 255 || !ss)
		cerr << "Wrong value of -b" << endl;
}

void ParmParser::parseLimit(stringstream & ss)
{
	ss >> parms.parallelLimit;
	if (parms.parallelLimit <= 0 || !ss)
		cerr << "Wrong value of -l" << endl;
}

void ParmParser::parseCount(stringstream & ss)
{
	ss >> parms.reqImageCount;
	if (parms.reqImageCount <= 0 || !ss)
		cerr << "Wrong value of -k" << endl;
}

void ParmParser::parseLogFilename(stringstream & ss) 
{
	ss >> parms.logFilename;
	if (!ss)
		cerr << "Wrong value of -f" << endl;
}

void ParmParser::checkRequirements()
{
	if (parms.reqBrightness == -1)
	{
		cout << "Parameter -b is required" << endl;
		return;
	}

	if (parms.reqImageCount == -1)
	{
		cout << "Parameter -k is required" << endl;
		return;
	}

	parms.good = true;
}