#ifndef ApplicationAttributes_hpp
#define ApplicationAttributes_hpp

#include <stdio.h>

class ApplicationArguments {
private:
    unsigned int brightness;
    unsigned int image_numbers;
    const char* log_file_name;
    
    int findKey(char* key, int argc, const char* argv[]);
public:
    void extract(int argc, const char * argv[]);
    int getBrightness();
    int getImageNumbers();
    const char* getLogFileName();
};

#endif
