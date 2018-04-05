#pragma once

#include <options.h>

namespace flow
{
    class args
    {
        options::ArgParser arg_parser_;

    public:
        explicit args();

        void parse(int argc, char** argv);

        bool input_file_path(std::string& nodes, std::string& edges);

        bool output_file_path(std::string& output);

        bool remove_first_line();

        int max_iterations();

        double accuracy();

        bool verbose();

        void help();
    };
}