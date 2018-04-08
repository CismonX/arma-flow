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
        /**
         * Determines width of stdout.
         * 
         * @return Character width.
         */
        static int max_elems_per_line();

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
         * Print a matrix to stdout.
         * 
         * @param mat Matrix to be printed.
         */
        static void print_mat(const arma::dmat& mat);
    };
}