#pragma once

#include <armadillo>
#include <vector>

namespace flow
{
    class calc
    {
        struct node_data
        {
            unsigned long offset;
            double v, g, p, q;
            enum node_type {
                pq, pv, swing
            } type;
        };

        struct edge_data
        {
            node_data *n1, *n2;
            double r, x, b, k;
            std::complex<double> impedance() const;
            std::complex<double> grounding_admittance() const;
        };

        std::vector<node_data> nodes_;

        unsigned long node_num_ = 0;

        std::vector<edge_data> edges_;

        arma::dmat node_adm_real_;

        arma::dmat node_adm_imag_;

    public:
        explicit calc() = default;

        void init(const arma::dmat& nodes, const arma::dmat& edges,
            int max_iterations, double accuracy);

        void node_admittance();

        arma::dmat* get_node_adm_real()
        {
            return &node_adm_real_;
        }

        arma::dmat* get_node_adm_imag()
        {
            return &node_adm_imag_;
        }
    };
}