#include "stdafx.h"
#include "Arg_parser.h"


Arg_parser::Arg_parser(int argc, char *argv[])
{

	for (int i = 1; i < argc; i += 2) {
	if (!std::strcmp(argv[i], "-b")) {
			brightness = atoi(argv[i + 1]);
			
		}
		else if (!std::strcmp(argv[i], "-l")) {
			image_limit = atoi(argv[i + 1]);
			
		}
		else if (!std::strcmp(argv[i], "-n")) {
			number_image = atoi(argv[i + 1]);
			
		}
		else if (!std::strcmp(argv[i], "-f")) {
			file_name = argv[i + 1];
			
		}	
	}

	
	
}


Arg_parser::~Arg_parser()
{
}
