#ifndef _PROGRAM_SETTINGS_H_
#define _PROGRAM_SETTINGS_H_

#include <string>

struct ProgramSettings
{
    int image_count;
    int image_width;
    int image_height;
    int flow_limit;
    std::string log_file;
    int certain_brightness;

    ProgramSettings();
    bool parse(int argc, const char** argv);
};

#endif // _PROGRAM_SETTINGS_H_
