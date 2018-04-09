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

            /// Gounding admittance (imaginary).
            double b;

            /// Transformer ratio.
            double k;

            /**
             * Get impedance of edge.
             * 
             * @return Impedance (complex).
             */
            std::complex<double> impedance() const
            {
                const auto deno = r * r + x * x;
                return { r / deno, -x / deno };
            }

            /**
             * Get gounding admittance.
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
        arma::dmat n_adm_g_, n_adm_b_;

        /// Given values of power and voltage.
        arma::colvec init_p_, init_q_, init_v_;
        
        /// Correction vector of voltage.
        arma::colvec e_, f_;

        /// Imbalace of active/reactive power and voltage.
        arma::colvec delta_p_, delta_q_, delta_v_;

        /// Submatrix of Jacobian matrix.
        arma::dmat j_h_, j_n_, j_m_, j_l_, j_r_, j_s_;

        /// F(x) of jacobian matrix.
        arma::colvec f_x_;

        /// Jacobian matrix (sparse).
        arma::sp_dmat j_;

        /// Power vector of nodes.
        arma::colvec p_, q_;

        /// Whether verbose output is enabled.
        bool verbose_ = false;

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
            for (auto row = 0U; row < mat.n_rows; ++row)
                for (auto col = 0U; col < mat.n_cols; ++col)
                    func(mat.at(row, col), row, col);
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
            for (auto i = 0U; i < vec.n_elem; ++i)
                func(vec[i], i);
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
        void init(const arma::dmat& nodes, const arma::dmat& edges,
            bool verbose, double epsilon);

        /**
         * Calculate node admittance.
         */
        std::pair<arma::dmat, arma::dmat> node_admittance();

        /**
         * Initialize iteratiob.
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
         * Get result of calculation
         * 
         */
        arma::dmat result();
    };
}
