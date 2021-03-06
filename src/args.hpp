//
// arma-flow/args.hpp
//
// @author CismonX
//

#pragma once

#include <complex>
#include <janus.h>

namespace flow
{
    /// Provides CLI argument parsing utility.
    class args
    {
        /// The argument parser.
        janus::ArgParser arg_parser_;

    public:
        /**
         * Constructor.
         */
        explicit args();

        /**
         * Parse arguments.
         * 
         * @param argc Number of arguments.
         * @param argv Arguments.
         */
        void parse(int argc, char** argv);

        /**
         * Get input file path from arguments.
         * 
         * @param nodes Path to node data file.
         * @param edges Path to edge data file.
         * @return Whether argument is provided.
         */
        bool input_file_path(std::string& nodes, std::string& edges);
        
        /**
         * Get output file path prefix from arguments.
         * 
         * @param output Path prefix to output files.
         * @return Whether argument is provided.
         */
        bool output_file_path(std::string& output);

        /**
         * Check whether to remove the first line from input files.
         */
        bool remove_first_line();

        /**
         * Check max number of iterations before aborting calculation.
         * 
         * @return Whether argument is provided.
         */
        bool max_iterations(unsigned& max);

        /**
         * Check max deviation to be tolerated.
         * 
         * @return Whether argument is provided.
         */
        bool accuracy(double& epsilon);

        /**
         * Calculate three-phase short circuit on specified node.
         * 
         * @return Whether argument is provided.
         */
        bool short_circuit(unsigned& node);

        /**
         * Check whether to ignore load current when calculating short circuit.
         */
        bool ignore_load();

        /**
         * Get transition impedance of three-phase short circuit.
         */
        bool transition_impedance(std::complex<double>& z_f);

        /**
         * Check whether to enable verbose output.
         * 
         * @return Whether argument is provided.
         */
        bool verbose();

        /**
         * Print help message and exit.
         * 
         * @return Whether argument is provided.
         */
        void help();
    };
}