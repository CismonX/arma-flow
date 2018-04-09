//
// arma-flow/args.cpp
//
// @author CismonX
//

#include "args.hpp"

namespace flow
{
    args::args() : arg_parser_(
        "A simple power flow calculator using Newton's method.\n"
        "usage: arma-flow [--version] [-h | --help] [-o <output_file_prefix>]\n"
        "                 -n <node_data_file> -e <edge_data_file> [-r]\n"
        "                 [-i <max_iterations>] [-a <accuracy>]\n"
        "                 [-v | --verbose]",
        "arma-flow version 0.0.1")
    {
        arg_parser_.newString("o", "result-");
        arg_parser_.newString("n");
        arg_parser_.newString("e");
        arg_parser_.newFlag("r");
        arg_parser_.newInt("i", 100);
        arg_parser_.newDouble("a", 0.00001);
        arg_parser_.newFlag("verbose v");
    }

    bool args::input_file_path(std::string& nodes, std::string& edges)
    {
        if (!arg_parser_.found("n") || !arg_parser_.found("e"))
            return false;
        nodes = arg_parser_.getString("n");
        edges = arg_parser_.getString("e");
        return true;
    }

    bool args::output_file_path(std::string& output)
    {
        output = arg_parser_.getString("o");
        return arg_parser_.found("o");
    }

    bool args::remove_first_line()
    {
        return arg_parser_.getFlag("r");
    }

    bool args::max_iterations(unsigned& max)
    {
        const auto arg_i = arg_parser_.getInt("i");
        if (!arg_parser_.found("i") || arg_i <= 0) {
            max = 100;
            return false;
        }
        max = arg_i;
        return true;
    }

    bool args::accuracy(double& epsilon)
    {
        epsilon = arg_parser_.getDouble("a");
        return arg_parser_.found("a");
    }

    bool args::verbose()
    {
        return arg_parser_.getFlag("v");
    }

    void args::help()
    {
        arg_parser_.exitHelp();
    }

    void args::parse(int argc, char** argv)
    {
        arg_parser_.parse(argc, argv);
    }
}
