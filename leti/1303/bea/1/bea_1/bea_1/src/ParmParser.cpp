#include "ParmParser.h"

Params parseParams(int argc, char ** argv)
{
	Params params;
	stringstream ss;

	for (int i = 0; i < argc; i++)
		ss << argv[i] << ' ';

	while (!ss.eof())
	{
		string parmName;
		ss >> parmName;

		// parse '-b'
		if (parmName == "-b")
		{
			ss >> params.b;
			if (params.b < 0 || params.b > 255 || !ss)
			{
				cerr << "The value of -b is invalid" << endl;
				return params;
			}
			continue;
		}

		// parse '-l'
		if (parmName == "-l")
		{
			ss >> params.l;
			if (params.l <= 0 || params.l > 10 || !ss)
			{
				cerr << "The value of -l is invalid. Only values from 1 to 10 are allowed" << endl;
				return params;
			}
			continue;
		}

		// parse '-k'
		if (parmName == "-k")
		{
			ss >> params.k;
			if (params.k <= 0 || !ss)
			{
				cerr << "The value of -k is invalid" << endl;
				return params;
			}
			continue;
		}

		// parse '-size'
		if (parmName == "-size")
		{
			ss >> params.m >> params.n;
			if (params.m < 0 || params.n < 0 || !ss)
			{
				cerr << "The values of -size are invalid" << endl;
				return params;
			}
			continue;
		}

		// parse '-f'
		if (parmName == "-f")
		{
			ss >> params.filename;
			if (!ss)
			{
				cerr << "The value of -f is invalid" << endl;
				return params;
			}
			continue;
		}
	}

	if (params.b == -1)
	{
		cerr << "Parameter -b is required" << endl;
		return params;
	}

	if (params.k == 0)
	{
		cerr << "Parameter -k is required" << endl;
		return params;
	}

	cout << "Parameters summury" << endl;
	cout << "    Target brightness: " << params.b << endl;
	cout << "    Number of threads: " << params.l << endl;
	cout << "    Number of images:  " << params.k << endl;
	cout << "    Size of images:    " << params.m << " x " << params.n << endl;
	if (params.filename == "")
		cout << "    Logging desabled" << endl;
	else
		cout << "    Log file:          '" << params.filename << "'" << endl;


	params.ok = true;
	return params;
}