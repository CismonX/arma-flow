//
// arma-flow/executor.cpp
//
// @author CismonX
//

#include <iostream>

#include "executor.hpp"
#include "factory.hpp"
#include "writer.hpp"

namespace flow
{
    void executor::execute(int argc, char** argv) const
    {
        // Get componenets.
        auto factory = factory::get();
        auto args = factory->get_args();
        auto input = factory->get_reader();
        auto calc = factory->get_calc();

        // Parse options.
        args->parse(argc, argv);

        // Read data from file.
        std::string path_to_nodes, path_to_edges;
        if (!args->input_file_path(path_to_nodes, path_to_edges))
            args->help();
        const auto remove = args->remove_first_line();
        if (!input->from_csv_file(path_to_nodes, remove))
            writer::error("Failed to read node data from file.");
        const auto nodes = input->get_mat();
        if (!input->from_csv_file(path_to_edges, remove))
            writer::error("Failed to read edge data from file.");
        const auto edges = input->get_mat();

        // Get options.
        const auto verbose = args->verbose();
        auto max = 0U;
        if (!args->max_iterations(max) && verbose)
            writer::notice("Max number of iterations not specified. Defaulted to 100.");
        auto epsilon = 0.0;
        if (!args->accuracy(epsilon) && verbose)
            writer::notice("Accuracy not specified. Defaulted to 0.00001.");
        if (epsilon < 0 || epsilon > 1)
            writer::error("Invalid accuracy.");
        std::string output_path;
        if (!args->output_file_path(output_path) && verbose)
            writer::notice("Output file path not specified. Defaulted to result-*.csv.");

        // Initialize calculation.
        calc->init(nodes, edges, verbose, epsilon);
        calc->node_admittance();
        calc->iterate_init();

        // Do iteration.
        unsigned num_iterations;
        do {
            num_iterations = calc->solve();
            if (calc->get_max() <= epsilon) {
                writer::println("Finished. Total number of iterations: ", num_iterations);
                const auto result = calc->result();
                if (verbose) {
                    writer::println("Result(n, P, Q, V, theta):");
                    writer::print_mat(result);
                }
                break;
            }
        } while (num_iterations < max);
        writer::println("Exceeds max number of iterations. Aborted.");
    }
}
