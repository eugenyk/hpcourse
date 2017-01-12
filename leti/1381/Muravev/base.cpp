#include "base.h"
#include <stdexcept>
#include <iostream>
using namespace std;
bool parse(int argc, char** argv, base& args)
{
    int it = 1;
    string buf, buf2;
    while(it < argc) 
	{
		buf = argv[it];		
        if(it++ == argc)
            return false;
		buf2 = argv[it];
        it++;
        try {
			if (buf == "-h")
				args.height = std::stoi(buf2);
			else if (buf == "-w")
				args.width = std::stoi(buf2);
			else if (buf == "-b")
				args.spec = std::stoi(buf2);
			else if (buf == "-l")
				args.imagSizeMax = std::stoi(buf2);
			else 
				if (buf == "-f") 
				{
					args.logData = true;
					if (buf2 != "")
						args.fileName = buf2;
					else
						return 0;
				}
				else
					return 0;
        } 
		catch(invalid_argument& e) 
		{
			cout << "Misstake: " << e.what() << endl;
                return 0;
        }
    }
	if (args.imagSizeMax <= 0 || args.width <= 0 || args.height <= 0)
        return 0;
    return 1;
}