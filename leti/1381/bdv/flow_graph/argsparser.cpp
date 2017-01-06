#include "argsparser.h"

ArgsParser::ArgsParser()
{    
}

bool ArgsParser::parse(int argc, char **argv, graph_options &opt)
{
    int i = 1;
    std::string s, s_;
    while(i < argc)
    {
        s = argv[i];
        i++;
        if(i == argc)
            return false;
        s_ = argv[i];
        i++;
        try {
            if(s == "-h")
                opt.image_h = std::stoi(s_);
            else if(s == "-w")
                opt.image_w = std::stoi(s_);
            else if(s == "-b")
                opt.brightness = std::stoi(s_);
            else if (s == "-l")
                opt.max_images = std::stoi(s_);
            else if (s == "-f")
            {
                opt.logging = true;
                if(s_ != "")
                    opt.filename = s_;
                else
                    return false;
            }
            else
                return false;
        } catch(std::invalid_argument& e) {
                return false;
        }
    }
    if(opt.image_h <= 0 || opt.image_w <= 0
            || opt.max_images <= 0)
        return false;
    return true;
}
