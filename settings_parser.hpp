#pragma once
#include <iostream>
#include <unordered_map>
#include <string>

namespace settings_parser
{

struct my_settings_t
{
    bool verbose = false;
    bool progress_bar = false;

    bool test_LFU = true;
    bool test_perfect = true;

    void parse_settings(int argc, const char* argv[])
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string str = argv[i];
            if (str == "-V") // (with dump of structures)
            {
                verbose = true;
            }
            else if (str == "-P") // (with progress bar, seems to work only in windows cmd)
            {
                progress_bar = true;
            }
            else if (str == "-no_LFU") // (do not test LFU cache)
            {
                test_LFU = false;
            }
            else if (str == "-no_perfect") // (do not test perfect cache)
            {
                test_perfect = false;
            }
            else
            {
                std::cout << "Unknown flag \"" << str << "\"" << std::endl;
                exit(0);
            }
        }
    }

    void print_settings() const
    {
        std::cout << "\nverbose: "      << verbose <<
                     "\nprogress_bar: " << progress_bar <<
                     "\ntest_LFU: "     << test_LFU <<
                     "\ntest_perfect: " << test_perfect << std::endl;
    }
};

} // namespace settings_parser
