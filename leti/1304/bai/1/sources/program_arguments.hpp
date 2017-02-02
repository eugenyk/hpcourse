#ifndef __HPCOURSE_LAB1_PROGRAM_ARGUMENTS_HPP__
#define __HPCOURSE_LAB1_PROGRAM_ARGUMENTS_HPP__

#include <stdint.h>
#include <string>

namespace hpcourse {

class ProgramArguments {
public:

    ProgramArguments(int argc, const char** argv);

    ~ProgramArguments();

    ::uint32_t& brightness();

    ::uint32_t& threads();

    ::std::string& file();

private:

    ProgramArguments(const hpcourse::ProgramArguments&) = delete;
    ProgramArguments(hpcourse::ProgramArguments&&) = delete;
    hpcourse::ProgramArguments& operator=(const hpcourse::ProgramArguments&) = delete;
    hpcourse::ProgramArguments& operator=(hpcourse::ProgramArguments&&) = delete;

    ::uint32_t m_brightness {0};
    ::uint32_t m_threads_count {2};
    ::std::string m_log_file_name {"hpcourse_lab1.txt"};

};

} // namespace hpcourse

#endif /* __HPCOURSE_LAB1_PROGRAM_ARGUMENTS_HPP__ */
