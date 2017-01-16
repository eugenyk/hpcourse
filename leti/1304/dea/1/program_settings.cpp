#include "program_settings.h"
#include <cstring>

ProgramSettings::ProgramSettings() : image_count(50), image_width(100), image_height(100),
    flow_limit(5), certain_brightness(0)
{
}

bool ProgramSettings::parse(int argc, const char** argv)
{
    image_width = -1;
    image_height = -1;
    for(int i =1; i < argc; i += 2)
    {
        if ((strcmp(argv[i], "-n") == 0))
        {
            image_count = std::atoi(argv[i+1]);
        }
        else if ((strcmp(argv[i], "-w") == 0))
        {
            image_width = std::atoi(argv[i+1]);
        }
        else if ((strcmp(argv[i], "-h") == 0))
        {
            image_height = std::atoi(argv[i+1]);
        }
        else if ((strcmp(argv[i], "-l") == 0))
        {
            flow_limit = std::atoi(argv[i+1]);
        }
        else if ((strcmp(argv[i], "-b") == 0))
        {
            certain_brightness = std::atoi(argv[i+1]);
        }
        else if ((strcmp(argv[i], "-f") == 0))
        {
            log_file = argv[i+1];
        }
        else
        {
            return false;
        }
    }
    if (image_width < 0)
    {
        if (image_height < 0)
        {
            image_height = 100;
            image_width = 100;
        }
        else
        {
            image_width = image_height;
        }
    }
    else
    {
        if (image_height < 0)
        {
            image_height = image_width;
        }
    }
    return true;
}
