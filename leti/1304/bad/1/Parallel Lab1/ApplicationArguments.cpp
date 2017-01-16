#include "ApplicationArguments.hpp"
#include <iostream>
#include <string.h>

int ApplicationArguments::findKey(char *key, int argc, const char **argv) {
    for(int i = 0; i < argc; i++) {
        if(strcmp(key, argv[i]) == 0) {
            return i;
        }
    }
    return -2;
}

void ApplicationArguments::extract(int argc, const char **argv) {
    int i = -1;
    
    i = this->findKey("-b", argc, argv);
    this->brightness = std::stoi(argv[i + 1]);
    
    i = this->findKey("-l", argc, argv);
    this->image_numbers = std::stoi(argv[i + 1]);
    
    i = findKey("-f", argc, argv);
    this->log_file_name = argv[i + 1];
}

int ApplicationArguments::getBrightness() {
    return this->brightness;
}

int ApplicationArguments::getImageNumbers() {
    return this->image_numbers;
}

const char* ApplicationArguments::getLogFileName() {
    return this->log_file_name;
}
