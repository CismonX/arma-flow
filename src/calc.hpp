//
// arma-flow/calc.hpp
//
// @author CismonX
//

#pragma once

#include <armadillo>
#include <vector>

namespace flow
{
    /// Power flow calculation.
    class calc
    {
        /// Structure of node data.
        struct node_data
        {
            /// Node ID.
            unsigned id;

            /// Voltage (real);
            double v;

            /// Generator (active power)
            double g;

            /// Load (active power)
            double p;

            /// Load (reactive power)
            double q;

            /// Generator admittance
            double x_d;

            /// Node type.
            enum node_type {
                pq, pv, swing
            } type;
        };

        /// Structure of edge data.
        struct edge_data
        {
            /// First and second node ID.
            unsigned m, n;

            /// Resistance (real).
            double r;

            /// Resistance (imaginary).
            double x;

            /// Grounding admittance (imaginary).
            double b;

            /// Transformer ratio.
            double k;

            /**
             * Get admittance of edge.
             * 
             * @return Admittance (complex).
             */
            std::complex<double> admittance() const
            {
                const auto deno = r * r + x * x;
                return { r / deno, -x / deno };
            }

            /**
             * Get impedance of edge.
             *
             * @return Admittance (complex).
             */
            std::complex<double> impedance() const
            {
                return { r, x };
            }

            /**
             * Get grounding admittance.
             * 
             * @return Grounding admittance (complex).
             */
            std::complex<double> grounding_admittance() const
            {
                return { 0, b };
            };
        };

        /// Vector of nodes.
        std::vector<node_data> nodes_;

        /// Number of nodes.
        unsigned num_nodes_ = 0;

        /// Number of PQ nodes and PV nodes.
        unsigned num_pq_ = 0, num_pv_ = 0;

        /// Vector of edges.
        std::vector<edge_data> edges_;

        /// Adjacency matrix of nodes.
        arma::uchar_mat adj_;

        /// Node admittance matrix.
        arma::cx_mat n_adm_;

        /// Node admittance matrix in original node order.
        arma::cx_mat n_adm_orig_;

        /// Node admittance matrix.
        arma::mat n_adm_g_, n_adm_b_;

        /// Node impedance matrix.
        arma::cx_mat n_imp_;

        /// Node impedance matrix.
        arma::mat n_imp_g_, n_imp_b_;

        /// Given values of power and voltage.
        arma::colvec init_p_, init_q_, init_v_;
        
        /// Correction vector of voltage.
        arma::colvec e_, f_;

        /// Imbalance of active/reactive power and voltage.
        arma::colvec delta_p_, delta_q_, delta_v_;

        /// Submatrix of Jacobian matrix.
        arma::mat j_h_, j_n_, j_m_, j_l_, j_r_, j_s_;

        /// F(x) of jacobian matrix.
        arma::colvec f_x_;

        /// Jacobian matrix (sparse).
        arma::sp_mat j_;

        /// Power vector of nodes.
        arma::colvec p_, q_;

        /// Voltage of nodes.
        arma::colvec v_;

        /// Whether to calculate short circuit.
        bool short_circuit_;

        /// Whether to ignore load current when calculating short circuit.
        bool ignore_load_;

        /// Node ID of three-phase short circuit.
        unsigned short_circuit_node_;

        /// Transition impedance of node.
        std::complex<double> z_f_;

        /// Value of short circuit current.
        std::complex<double> i_f_;

        /// Vector of short circuit voltage.
        arma::cx_colvec u_f_;

        /// Whether verbose output is enabled.
        bool verbose_;

        /// Max deviation to be tolerated.
        double epsilon_ = 0;

        /// Number of iterations.
        unsigned n_iter_ = 1;
        
        /**
         * Get offset of sorted node by ID.
         * 
         * @param id Node ID.
         * @return Node offset.
         */
        unsigned node_offset(unsigned id) const;

        /// G(i, j) * e(i) + B(i, j) * f(i)
        double j_elem_g_b(unsigned row, unsigned col) const;

        /// B(i, j) * e(i) - G(i, j) * f(i)
        double j_elem_b_g(unsigned row, unsigned col) const;

        /// G(i, i) * e(i) + B(i, i) * f(i)
        double j_elem_b(unsigned row) const
        {
            return j_elem_g_b(row, row);
        }

        /// B(i, i) * e(i) - G(i, i) * f(i)
        double j_elem_d(unsigned row) const
        {
            return j_elem_b_g(row, row);
        }

        /// sum(i == j || adj(i, j), B(i, j) * f(j) - G(i, j) * e(j))
        double j_elem_a(unsigned row) const;
        
        /// sum(i == j || adj(i, j), G(i, j) * f(j) + B(i, j) * e(j))
        double j_elem_c(unsigned row) const;

        /**
         * Traverse a matrix.
         * 
         * @param mat Matrix to be traversed.
         * @param func Callback for each element.
         */
        template <typename M, typename F>
        static void mat_elem_foreach(M& mat, F func)
        {
            for (auto row = 0U; row < mat.n_rows; ++row) {
                for (auto col = 0U; col < mat.n_cols; ++col) {
                    func(mat.at(row, col), row, col);
                }
            }
        }

        /**
         * Traverse a vector.
         * 
         * @param vec Vector to be traversed.
         * @param func Callback for each element
         */
        template <typename V, typename F>
        static void vec_elem_foreach(V& vec, F func)
        {
            for (auto i = 0U; i < vec.n_elem; ++i) {
                func(vec[i], i);
            }
        }

        /**
         * Update F(x) of jacobian matrix.
         */
        void update_f_x();

        /**
         * Calculate jacobian matrix.
         */
        void jacobian();

        /**
         * Prepare to solve the formula.
         */
        void prepare_solve();

        /// Calculate active power of a node.
        double calc_p(unsigned row) const
        {
            return f_[row] * j_elem_c(row) - e_[row] * j_elem_a(row);
        }

        /// Calculate reactive power of a node.
        double calc_q(unsigned row) const
        {
            return -f_[row] * j_elem_a(row) - e_[row] * j_elem_c(row);
        }

        /// Check whether a value is approximately zero.
        bool approx_zero(double val) const
        {
            return std::abs(val) <= epsilon_;
        }

    public:
        /**
         * Default constructor.
         */
        explicit calc() = default;

        /**
         * Initialize.
         */
        void init(
            const arma::mat&            nodes,
            const arma::mat&            edges,
            bool                        verbose,
            double                      epsilon,
            bool                        short_circuit,
            bool                        ignore_load,
            unsigned                    short_circuit_node,
            const std::complex<double>& z_f);

        /**
         * Calculate node admittance.
         */
        std::pair<arma::mat, arma::mat> node_admittance();

        /**
         * Calculate node impedance. 
         */
        std::pair<arma::mat, arma::mat> node_impedance();

        /**
         * Initialize iteration.
         */
        void iterate_init();

        /**
         * Solve formula.
         * 
         * @return number of iterations.
         */
        unsigned solve();

        /**
         * Get max value of correction vector.
         */
        double get_max() const;

        /**
         * Get result of power flow calculation.
         */
        arma::mat result();

        /**
         * Get current of three-phase short circuit.
         */
        std::complex<double> short_circuit_current();

        /**
         * Get node voltage of short circuit.
         */
        arma::mat short_circuit_voltage();

        /**
         * Get edge current of short circuit.
         */
        arma::mat short_circuit_edge_current();
    };
}
