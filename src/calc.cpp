#include "calc.hpp"
#include "executor.hpp"

namespace flow
{
    std::complex<double> calc::edge_data::impedance() const
    {
        const auto deno = r * r + x * x;
        return { r / deno, -x / deno };
    }

    std::complex<double> calc::edge_data::grounding_admittance() const
    {
        return { 0, b };
    }

    void calc::init(const arma::dmat& nodes, const arma::dmat& edges,
        int max_iterations, double accuracy)
    {
        if (max_iterations <= 0)
            executor::print_and_exit("Bad number of iterations. Positive integer expected.");
        if (accuracy < 0 || accuracy > 1)
            executor::print_and_exit("Invalid accuracy.");
        if (nodes.n_cols != 5 || edges.n_cols != 6)
            executor::print_and_exit("Bad input matrix format.");
        nodes.each_row([this](const arma::rowvec& row)
        {
            using node_type = node_data::node_type;
            auto type = node_type::swing;
            switch (static_cast<unsigned>(row[4]))
            {
            case 0:
                break;
            case 1:
                type = node_type::pq;
                break;
            case 2:
                type = node_type::pv;
                break;
            default:
                executor::print_and_exit("Bad node type.");
            }
            nodes_.push_back({
                node_num_++, row[0], row[1], row[2], row[3], type
            });
        });
        edges.each_row([this](const arma::rowvec& row)
        {
            auto n1 = static_cast<unsigned>(row[0]) - 1;
            auto n2 = static_cast<unsigned>(row[1]) - 1;
            if (n1 >= node_num_ || n2 >= node_num_)
                executor::print_and_exit("Bad node offset.");
            edges_.push_back({
                &nodes_[n1], &nodes_[n2], row[2], row[3], row[4], row[5]
            });
        });
    }

    void calc::node_admittance()
    {
        arma::cx_mat node_adm_cplx(node_num_, node_num_, arma::fill::zeros);
        for (auto&& edge : edges_) {
            const auto n1 = edge.n1->offset;
            const auto n2 = edge.n2->offset;
            const auto impedance = edge.impedance();
            // Whether this edge has transformer.
            if (edge.k) {
                node_adm_cplx.at(n1, n1) += impedance;
                node_adm_cplx.at(n2, n2) += impedance / (edge.k * edge.k);
                node_adm_cplx.at(n1, n2) -= impedance / edge.k;
                node_adm_cplx.at(n2, n1) -= impedance / edge.k;
            }
            else {
                const auto delta_diag = impedance + edge.grounding_admittance();
                node_adm_cplx.at(n1, n1) += delta_diag;
                node_adm_cplx.at(n2, n2) += delta_diag;
                node_adm_cplx.at(n1, n2) -= impedance;
                node_adm_cplx.at(n2, n1) -= impedance;
            }
        }
        node_adm_real_ = arma::real(node_adm_cplx);
        node_adm_imag_ = arma::imag(node_adm_cplx);
    }
}
