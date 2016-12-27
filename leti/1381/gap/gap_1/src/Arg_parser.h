#pragma once

#include <iostream>
#include <string>
class Arg_parser
{
public:
	Arg_parser(int argc, char *argv[]);
	void print_help();
	~Arg_parser();
	unsigned char get_brightness(){ return brightness;}
	unsigned get_image_limit(){ return image_limit; }
	unsigned get_number_image(){ return number_image; }
	std::string get_file_name(){ return file_name; }
private:
	unsigned char brightness = 100;
	unsigned image_limit = 5;
	unsigned number_image = 10;
	std::string file_name = "";

};

