#pragma once

#include <armadillo>


namespace flow
{
    class output
    {
        static int max_elems_per_line();
    public:

        static void print_dmat(const arma::dmat& mat);

    };
}