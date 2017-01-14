#include "program_arguments.hpp"
#include <iostream>
#include <string>

hpcourse::ProgramArguments::ProgramArguments(int argc, const char** argv) {
    enum class TYPE {
        Nothing,
        Brightness,
        Threads,
        File
    } type {TYPE::Nothing};

    for(int i = 0; i < argc; i++) {
        ::std::string argument = *(argv + i);

        ::std::cout << "argv[" << i << "]: " << argument << "\n";

        if(argument == "-b") {
            type = TYPE::Brightness;
            continue;
        }
        else if(argument == "-l") {
            type = TYPE::Threads;
            continue;
        }
        else if(argument == "-f") {
            type = TYPE::File;
            continue;
        }

        switch (type) {
            case TYPE::Brightness:
                try {
                    ::uint32_t temp = stoi(argument);
                    if(temp <= 255) {
                        m_brightness = temp;
                    }
                    else {
                        throw true;
                    }
                }
                catch(...) {
                    std::cout << "Error. For brightness it will use the default value: " << m_brightness << "\n";
                }
                break;

            case TYPE::Threads:
                try {
                    ::uint32_t temp = stoi(argument);
                    m_threads_count = temp;
                }
                catch(...) {
                    std::cout << "Error. For threads limit it will use the default value: " << m_threads_count << "\n";
                }
                break;

            case TYPE::File:
                try {
                    m_log_file_name = argument;
                }
                catch(...) {
                    std::cout << "Error. For file name will use the default value: \"" << m_log_file_name << "\"\n";
                }
                break;

            default:
                break;
        }
    }

    std::cout << "\nBrightness: " << m_brightness << "\nThreads limit: " << m_threads_count << "\nFile name: " << m_log_file_name << "\n\n";
}

hpcourse::ProgramArguments::~ProgramArguments() {}

::uint32_t& hpcourse::ProgramArguments::brightness() {
    return m_brightness;
}

::uint32_t& hpcourse::ProgramArguments::threads() {
    return m_threads_count;
}

::std::string& hpcourse::ProgramArguments::file() {
    return m_log_file_name;
}
