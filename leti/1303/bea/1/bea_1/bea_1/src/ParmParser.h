#ifndef PARM_PARSER_H
#define PARM_PARSER_H

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

struct Params
{
	bool ok;
	int b;
	int l;
	int k;
	int n;
	int m;
	string filename;

	Params()
	{
		ok = false;
		b = -1;
		k = 0;

		m = n = 24;

		l = 1;
	}
};

Params parseParams(int argc, char ** argv);

#endif