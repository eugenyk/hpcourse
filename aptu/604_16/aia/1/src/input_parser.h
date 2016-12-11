#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H

#include <string>
#include <cstdlib>
#include <sstream>
#include <cstring>

namespace input_parser
{
    struct InputParser
    {
        InputParser(int argc, char* argv[])
                : m_targetBrightness(-1)
                , m_taskLimit(-1)
                , m_logName("")
        {
            parse(argc, argv);
        };


        bool isValid()
        {
            return (m_targetBrightness >= 0) && (m_targetBrightness < MAX_VALUE) && (m_taskLimit > 0) && !m_logName.empty();
        }

        void parse(int argc, char* argv[])
        {
            for (int i = 0; i < argc; ++i)
            {
                if (strcmp("-b", argv[i]) == 0 && i + 1 < argc)
                {
                    m_targetBrightness = std::stoi(argv[++i]);
                    continue;
                }
                if (strcmp("-l", argv[i]) == 0 && i + 1 < argc)
                {
                    m_taskLimit =  std::stoi(argv[++i]);
                    continue;
                }
                if (strcmp("-f", argv[i]) == 0 && i + 1 < argc)
                {
                    m_logName = argv[++i];
                    continue;
                }
            }
        }
        
        std::string toString() const
        {
            std::stringstream ss;
            ss << "Input target brightness = " << m_targetBrightness << std::endl;
            ss << "Input task limit        = " << m_taskLimit << std::endl;
            ss << "Input file name         = " << m_logName << std::endl;
            
            return ss.str();
        }

        static const int MAX_VALUE = 255;
        int m_targetBrightness;
        int m_taskLimit;
        std::string m_logName;
    };
}// input_parser
#endif /* INPUT_PARSER_H */

