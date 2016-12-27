#pragma once

#include <iostream>
#include <string>
class Data
{
public:
	Data(int argc, char *argv[]);
	void print_help();
	~Data();
	unsigned char get_brightness(){ return brightness; }
	unsigned get_image_limit(){ return image_limit; }
	unsigned get_number_image(){ return number_image; }
	std::string get_file_name(){ return file_name; }
private:
	unsigned char brightness = 128;
	unsigned image_limit = 7;
	unsigned number_image = 15;
	std::string file_name = "";

};

