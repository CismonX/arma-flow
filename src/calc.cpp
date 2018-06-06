//
// arma-flow/calc.cpp
//
// @author CismonX
//

#include "calc.hpp"
#include "writer.hpp"

namespace flow
{
    unsigned calc::node_offset(unsigned id) const
    {
        for (auto offset = 0U; offset < num_nodes_; ++offset)
            if (nodes_[offset].id == id)
                return offset;
        return 0;
    }

    double calc::j_elem_g_b(unsigned row, unsigned col) const
    {
        return n_adm_g_.at(row, col) * e_[row] + n_adm_b_.at(row, col) * f_[row];
    }

    double calc::j_elem_b_g(unsigned row, unsigned col) const
    {
        return n_adm_b_.at(row, col) * e_[row] - n_adm_g_.at(row, col) * f_[row];
    }

    double calc::j_elem_a(unsigned row) const
    {
        auto sum = 0.0;
        for (auto col = 0U; col < num_nodes_; ++col)
            if (row == col || adj_.at(row, col))
                sum += n_adm_b_.at(row, col) * f_[col] - n_adm_g_.at(row, col) * e_[col];
        return sum;
    }

    double calc::j_elem_c(unsigned row) const
    {
        auto sum = 0.0;
        for (auto col = 0U; col < num_nodes_; ++col)
            if (row == col || adj_.at(row, col))
                sum += n_adm_g_.at(row, col) * f_[col] + n_adm_b_.at(row, col) * e_[col];
        return sum;
    }

    void calc::jacobian()
    {
        mat_elem_foreach(j_h_, [this](auto& elem, auto row, auto col)
        {
            if (row == col)
                elem = j_elem_c(row) - j_elem_d(row);
            else
                elem = -j_elem_b_g(row, col);
        });
        mat_elem_foreach(j_n_, [this](auto& elem, auto row, auto col)
        {
            if (row == col)
                elem = -j_elem_a(row) + j_elem_b(row);
            else
                elem = j_elem_g_b(row, col);
        });
        mat_elem_foreach(j_m_, [this](auto& elem, auto row, auto col)
        {
            if (row == col)
                elem = -j_elem_a(row) - j_elem_b(row);
            else
                elem = -j_elem_g_b(row, col);
        });
        mat_elem_foreach(j_l_, [this](auto& elem, auto row, auto col)
        {
            if (row == col)
                elem = -j_elem_c(row) - j_elem_d(row);
            else
                elem = -j_elem_b_g(row, col);
        });
        // We shouldn't start at 0. We should start at m.
        mat_elem_foreach(j_r_, [this](auto& elem, auto row, auto col)
        {
            if (row + num_pq_ == col)
                elem = f_[row + num_pq_] * 2;
            else
                elem = 0;
        });
        mat_elem_foreach(j_s_, [this](auto& elem, auto row, auto col)
        {
            if (row + num_pq_ == col)
                elem = e_[row + num_pq_] * 2;
            else
                elem = 0;
        });
    }

    void calc::init(const arma::mat& nodes, const arma::mat& edges,
        bool verbose, double epsilon, bool short_circuit, bool ignore_load,
        unsigned short_circuit_node, const std::complex<double>& z_f)
    {
        if (nodes.n_cols != (short_circuit ? 6 : 5) || edges.n_cols != 6)
            writer::error("Bad input matrix format.");
        short_circuit_ = short_circuit;
        nodes.each_row([this](const arma::rowvec& row)
        {
            const auto type_val = static_cast<unsigned>(row[short_circuit_ ? 5 : 4]);
            auto type = node_data::swing;
            if (type_val == 1) {
                type = node_data::pq;
                ++num_pq_;
            } else if (type_val == 2) {
                type = node_data::pv;
                ++num_pv_;
            } else if (type_val != 0)
                writer::error("Bad node type.");
            if (short_circuit_) {
                nodes_.push_back({
                    num_nodes_++, row[0], row[1], row[2], row[3], row[4], type
                });
            } else {
                nodes_.push_back({
                    num_nodes_++, row[0], row[1], row[2], row[3], 0, type
                });
            }
        });
        // Nodes should be sorted, PQ nodes should be followed by PV nodes,
        // while swing node be the last.
        std::sort(nodes_.begin(), nodes_.end(), [](auto&& n1, auto&& n2)
        {
            return n1.type < n2.type;
        });
        adj_.zeros(num_nodes_, num_nodes_);
        if (num_nodes_ != num_pq_ + num_pv_ + 1)
            writer::error("Only one swing node should exist.");
        edges.each_row([this](const arma::rowvec& row)
        {
            const auto n1 = static_cast<unsigned>(row[0]) - 1;
            const auto n2 = static_cast<unsigned>(row[1]) - 1;
            if (n1 >= num_nodes_ || n2 >= num_nodes_)
                writer::error("Bad node offset.");
            edges_.push_back({
                n1, n2, row[2], row[3], row[4], row[5]
            });
        });
        verbose_ = verbose;
        epsilon_ = epsilon;
        ignore_load_ = ignore_load;
        if (short_circuit_node < 1 || short_circuit_node > num_nodes_)
            writer::error("Bad node ID for short circuit calculation.");
        short_circuit_node_ = node_offset(short_circuit_node - 1);
        z_f_ = z_f;
    }

    std::pair<arma::mat, arma::mat> calc::node_admittance()
    {
        n_adm_.zeros(num_nodes_, num_nodes_);
        n_adm_orig_.zeros(num_nodes_, num_nodes_);
        for (auto&& edge : edges_) {
            const auto admittance = edge.admittance();
            const auto m = node_offset(edge.m);
            const auto n = node_offset(edge.n);
            // Whether this edge has transformer.
            if (edge.k) {
                n_adm_.at(m, m) = n_adm_orig_.at(edge.m, edge.m) += admittance;
                n_adm_.at(n, n) = n_adm_orig_.at(edge.n, edge.n) += admittance / std::pow(edge.k, 2);
                n_adm_.at(m, n) = n_adm_orig_.at(edge.m, edge.n) -= admittance / edge.k;
                n_adm_.at(n, m) = n_adm_orig_.at(edge.n, edge.m) -= admittance / edge.k;
            } else {
                const auto delta_diag = admittance + edge.grounding_admittance();
                n_adm_.at(m, m) = n_adm_orig_.at(edge.m, edge.m) += delta_diag;
                n_adm_.at(n, n) = n_adm_orig_.at(edge.n, edge.n) += delta_diag;
                n_adm_.at(m, n) = n_adm_orig_.at(edge.m, edge.n) -= admittance;
                n_adm_.at(n, m) = n_adm_orig_.at(edge.n, edge.m) -= admittance;
            }
            adj_.at(m, n) = 1;
            adj_.at(n, m) = 1;
        }
        n_adm_g_ = arma::real(n_adm_);
        n_adm_b_ = arma::imag(n_adm_);
        const auto n_adm_orig_g = arma::real(n_adm_orig_);
        const auto n_adm_orig_b = arma::imag(n_adm_orig_);
        if (verbose_) {
            writer::println("Real part of node admittance matrix:");
            writer::print_mat(n_adm_orig_g);
            writer::println("Imaginary part of node admittance matrix:");
            writer::print_mat(n_adm_orig_b);
        }
        return { n_adm_orig_g, n_adm_orig_b };
    }

    std::pair<arma::mat, arma::mat> calc::node_impedance()
    {
        n_imp_.zeros(num_nodes_, num_nodes_);
        auto adm = n_adm_, adm_orig = n_adm_orig_;
        auto i = 0U;
        for (auto&& node : nodes_) {
            if (node.type == node_data::pq) {
                if (!ignore_load_) {
                    // Note that we should use P(LD) and Q(LD).
                    const auto impedance = std::complex<double>(-init_p_[i], init_q_[i]) / std::pow(v_[i], 2);
                    adm.at(i, i) = adm_orig.at(node.id, node.id) += impedance;
                }
            } else {
                adm.at(i, i) = adm_orig.at(node.id, node.id) -= std::complex<double>(0, 1 / node.x_d);
            }
            ++i;
        }
        n_imp_ = adm.i();
        n_imp_g_ = arma::real(n_imp_);
        n_imp_b_ = arma::imag(n_imp_);
        const auto imp_orig = adm_orig.i();
        const auto n_imp_orig_g = arma::real(imp_orig);
        const auto n_imp_orig_b = arma::imag(imp_orig);
        if (verbose_) {
            writer::println("Real part of node impedance matrix:");
            writer::print_mat(n_imp_orig_g);
            writer::println("Imaginary part of node impedance matrix:");
            writer::print_mat(n_imp_orig_b);
        }
        return { n_imp_orig_g, n_imp_orig_b };
    }

    void calc::iterate_init()
    {
        init_p_.zeros(num_nodes_ - 1);
        init_q_.zeros(num_pq_);
        init_v_.zeros(num_pv_);
        auto i_p = 0;
        auto i_q = 0;
        auto i_v = 0;
        e_.resize(num_nodes_);
        for (auto&& node : nodes_) {
            if (node.type == node_data::pq) {
                init_p_[i_p] = -node.p;
                init_q_[i_q++] = -node.q;
            } else if (node.type == node_data::pv) {
                init_p_[i_p] = node.g - node.p;
                init_v_[i_v++] = node.v;
            }
            e_[i_p++] = node.v;
        }
        f_.zeros(num_nodes_);
        j_h_.zeros(num_nodes_ - 1, num_nodes_ - 1);
        j_n_.zeros(num_nodes_ - 1, num_nodes_ - 1);
        j_m_.zeros(num_pq_, num_nodes_ - 1);
        j_l_.zeros(num_pq_, num_nodes_ - 1);
        j_r_.zeros(num_pv_, num_nodes_ - 1);
        j_s_.zeros(num_pv_, num_nodes_ - 1);
        delta_p_.zeros(num_nodes_ - 1);
        delta_q_.zeros(num_pq_);
        delta_v_.zeros(num_pv_);
        p_.zeros(num_nodes_);
        q_.zeros(num_nodes_);
        update_f_x();
    }

    void calc::prepare_solve()
    {
        // Cross-construct F(x) vector.
        f_x_.zeros(2 * num_nodes_ - 2);
        for (auto row = 0U; row < num_nodes_ - 1; ++row) {
            f_x_[2 * row] = delta_p_[row];
            f_x_[2 * row + 1] = row < num_pq_ ? delta_q_[row] : delta_v_[row - num_pq_];
        }
        // Cross-construct jacobian matrix.
        arma::mat jacobian(2 * num_nodes_ - 2, 2 * num_nodes_ - 2);
        for (auto row = 0U; row < num_nodes_ - 1; ++row)
            for (auto col = 0U; col < num_nodes_ - 1; ++col) {
                jacobian.at(2 * row, 2 * col) = j_h_.at(row, col);
                jacobian.at(2 * row, 2 * col + 1) = j_n_.at(row, col);
                if (row < num_pq_) {
                    jacobian.at(2 * row + 1, 2 * col) = j_m_.at(row, col);
                    jacobian.at(2 * row + 1, 2 * col + 1) = j_l_.at(row, col);
                } else {
                    jacobian.at(2 * row + 1, 2 * col) = j_r_.at(row - num_pq_, col);
                    jacobian.at(2 * row + 1, 2 * col + 1) = j_s_.at(row - num_pq_, col);
                }
            }
        if (verbose_) {
            writer::println("Jacobian matrix");
            writer::print_mat(jacobian);
        }
        // Transform into a sparse matrix for spsolve().
        j_ = jacobian;
    }

    void calc::update_f_x()
    {
        vec_elem_foreach(delta_p_, [this](auto& elem, auto row)
        {
            p_[row] = calc_p(row);
            elem = init_p_[row] - p_[row];
        });
        vec_elem_foreach(delta_q_, [this](auto& elem, auto row)
        {
            q_[row] = calc_q(row);
            elem = init_q_[row] - q_[row];
        });
        vec_elem_foreach(delta_v_, [this](auto& elem, auto row)
        {
            auto i = row + num_pq_;
            elem = std::pow(init_v_[row], 2) - std::pow(e_[i], 2) - std::pow(f_[i], 2);
        });
        if (verbose_) {
            writer::println("Delta P:");
            writer::print_mat(delta_p_.t());
            writer::println("Delta Q:");
            writer::print_mat(delta_q_.t());
            writer::println("Delta U^2:");
            writer::print_mat(delta_v_.t());
        }
    }

    unsigned calc::solve()
    {
        if (verbose_)
            writer::println("Number of iterations: ", n_iter_, " (begin)");
        jacobian();
        prepare_solve();
        const auto x_vec = spsolve(j_, f_x_, "superlu");
        vec_elem_foreach(f_, [&x_vec](auto& elem, auto row)
        {
            elem += x_vec.at(2 * row, 0);
        });
        vec_elem_foreach(e_, [&x_vec](auto& elem, auto row)
        {
            elem += x_vec.at(2 * row + 1, 0);
        });
        if (verbose_) {
            writer::println("Correction vector of voltage (real):");
            writer::print_mat(e_.t());
            writer::println("Correction vector of voltage (imaginary):");
            writer::print_mat(f_.t());
        }
        update_f_x();
        if (verbose_)
            writer::println("Number of iterations: ", n_iter_, " (end)");
        return n_iter_++;
    }

    double calc::get_max() const
    {
        arma::mat mat = join_cols(join_cols(delta_p_, delta_q_), delta_v_);
        auto max = 0.0;
        for (auto&& elem : mat)
            if (std::abs(elem) > max)
                max = std::abs(elem);
        return max;
    }

    arma::mat calc::result()
    {
        p_[num_nodes_ - 1] = calc_p(num_nodes_ - 1);
        for (auto&& elem : p_)
            if (approx_zero(elem))
                elem = 0;
        vec_elem_foreach(delta_v_, [this](auto& elem, auto row)
        {
            auto i = row + num_pq_;
            q_[i] = calc_q(i);
        });
        q_[num_nodes_ - 1] = calc_q(num_nodes_ - 1);
        for (auto&& elem : q_)
            if (approx_zero(elem))
                elem = 0;
        v_.zeros(num_nodes_);
        vec_elem_foreach(v_, [this](auto& elem, auto col)
        {
            elem = std::sqrt(std::pow(e_[col], 2) + std::pow(f_[col], 2));
            if (approx_zero(elem))
                elem = 0;
        });
        arma::colvec theta(num_nodes_);
        vec_elem_foreach(theta, [this](auto& elem, auto row)
        {
            elem = std::atan(f_[row] / e_[row]);
            if (approx_zero(elem))
                elem = 0;
        });
        arma::colvec node_id(num_nodes_);
        vec_elem_foreach(node_id, [this](auto& elem, auto row)
        {
            elem = nodes_[row].id;
        });
        arma::mat retval = join_rows(node_id, join_rows(join_rows(v_, theta), join_rows(p_, q_)));
        // We shall preserve the original node sequence.
        arma::mat sorted_retval(num_nodes_, 4);
        retval.each_row([&sorted_retval](const arma::rowvec& row)
        {
            sorted_retval.row(row[0]) = row.subvec(1, row.n_elem - 1);
        });
        return sorted_retval;
    }

    std::complex<double> calc::short_circuit_current()
    {
        const auto n = short_circuit_node_;
        if (ignore_load_)
            i_f_ = 1;
        else
            i_f_ = { e_[n], f_[n] };
        i_f_ /= std::complex<double>(n_imp_g_.at(n, n), n_imp_b_.at(n, n)) + z_f_;
        return i_f_;
    }

    arma::mat calc::short_circuit_voltage()
    {
        u_f_.resize(num_nodes_);
        using cx = std::complex<double>;
        const auto n = short_circuit_node_;
        vec_elem_foreach(u_f_, [n, this](auto& elem, auto row)
        {
            std::complex<double> u_f(1), u_i(1);
            if (!ignore_load_) {
                u_f = { e_[row], f_[row] };
                u_i = { e_[n], f_[n] };
            }
            elem = u_f - cx(n_imp_g_.at(row, n), n_imp_b_.at(row, n)) *
                (u_i / (cx(n_imp_g_.at(n, n), n_imp_b_.at(n, n)) + z_f_));
            if (approx_zero(elem.real()))
                elem.real(0);
            if (approx_zero(elem.imag()))
                elem.imag(0);
        });
        arma::cx_colvec u_f_orig(num_nodes_);
        vec_elem_foreach(u_f_, [&u_f_orig, this](auto&& elem, auto row)
        {
            u_f_orig[nodes_[row].id] = elem;
        });
        if (verbose_) {
            writer::println("Short circuit node voltage:");
            for (auto&& elem : u_f_orig)
                writer::print_complex("", elem);
        }
        return join_rows(arma::real(u_f_), arma::imag(u_f_));
    }

    arma::mat calc::short_circuit_edge_current()
    {
        arma::cx_vec edge_current(edges_.size());
        auto i = 0U;
        if (verbose_)
            writer::println("Short circuit edge current:");
        for (auto&& edge : edges_)
        {
            const auto admittance = edge.admittance();
            const auto m = node_offset(edge.m);
            const auto n = node_offset(edge.n);
            std::complex<double> y;
            if (edge.k) {
                y = admittance * (edge.k * edge.k - edge.k + 1) / (edge.k * edge.k);
            } else {
                y = admittance + edge.grounding_admittance() * 2.0;
            }
            edge_current[i] = (u_f_[m] - u_f_[n] / (edge.k ? edge.k : 1)) * y;
            if (verbose_)
                writer::print_complex(std::to_string(edge.m + 1) + ',' +
                    std::to_string(edge.n + 1) + ": ", edge_current[i]);
            ++i;
        }
        return join_rows(arma::real(edge_current), arma::imag(edge_current));
    }
}
