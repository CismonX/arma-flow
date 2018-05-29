//
// arma-flow/writer.hpp
//
// @author CismonX
//

#pragma once

#include <armadillo>

namespace flow
{
    /// Provides write utilities.
    class writer
    {
        /// Prefix of output file path.
        std::string output_path_prefix_;

        /**
         * Determines width of stdout.
         * 
         * @return Character width.
         */
        static int max_elems_per_line();

        /**
         * Convert double to pretty string.
         * 
         * @param val Value to be converted.
         * @return Converted string.
         */
        static std::string double_to_string(double val);

    public:
        /**
         * Print a line to stdout.
         * 
         * @param message Messages to be printed.
         */
        template <typename ...T>
        static void println(T&&... message)
        {
            (std::cout << ... << message) << std::endl;
        }

        /**
         * Print a notice message to stdout.
         * 
         * @param message Messages to be printed.
         */
        template <typename ...T>
        static void notice(T&&... message)
        {
            (std::cout << "Notice: " << ... << message) << std::endl;
        }

        /**
         * Print a error message to stdout and terminate proogram.
         * 
         * @param message Messages to be printed.
         */
        template <typename ...T>
        static void error(T&&... message)
        {
            (std::cout << "Error: " << ... << message) << std::endl;
            exit(1);
        }

        /**
         * Set output path prefix.
         * 
         * @param prefix Output path prefix.
         */
        void set_output_path_prefix(const std::string& prefix)
        {
            output_path_prefix_ = prefix;
        }

        /**
         * Print a matrix to stdout.
         * 
         * @param mat Matrix to be printed.
         */
        static void print_mat(const arma::mat& mat);

        /**
         * Write a matrix to a file in CSV format.
         *
         * @param path Path to CSV file.
         * @param mat Matrix to be printed.
         * @param header Header of CSV file
         */
        bool to_csv_file(const std::string& path, const arma::mat& mat, const std::string& header = "");
    };
}