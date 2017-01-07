#ifndef PARM_PARSER_H
#define PARM_PARSER_H

#include "Parameters.h"

class ParmParser
{
private:
	Parameters parms;

	void parseBrightness(std::stringstream & ss);
	void parseLimit(std::stringstream & ss);
	void parseCount(std::stringstream & ss);
	void parseLogFilename(std::stringstream & ss);
	void checkRequirements();

public:
	Parameters parse(int argc, char * argv[]);
};

#endif