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
        "                 [-i <max_iterations>] [-a <accuracy>] [-v | --verbose]\n"
        "                 [-s <node_id>] [--ignore-load]\n"
        "                 [--tr <transition_impedance(real)>] [--ti <transition_impedance(imag)>]",
        "arma-flow version 0.0.1")
    {
        arg_parser_.newString("o", "result-");
        arg_parser_.newString("n");
        arg_parser_.newString("e");
        arg_parser_.newFlag("r");
        arg_parser_.newInt("i", 100);
        arg_parser_.newDouble("a", 0.00001);
        arg_parser_.newInt("s");
        arg_parser_.newFlag("ignore-load");
        arg_parser_.newDouble("tr", 0);
        arg_parser_.newDouble("ti", 0);
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

    bool args::short_circuit(unsigned& node)
    {
        if (!arg_parser_.found("s"))
            return false;
        node = arg_parser_.getInt("s");
        return true;
    }

    bool args::ignore_load()
    {
        return arg_parser_.getFlag("ignore-load");
    }

    bool args::transition_impedance(std::complex<double>& z_f)
    {
        if (!arg_parser_.found("tr") && !arg_parser_.found("ti"))
            return false;
        z_f = { arg_parser_.getDouble("tr"), arg_parser_.getDouble("ti") };
        return true;
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
