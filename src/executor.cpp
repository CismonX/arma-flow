
#include <iostream>

#include "executor.hpp"
#include "factory.hpp"
#include "output.hpp"

namespace flow
{
    void executor::print_and_exit(const std::string& message)
    {
        std::cout << message << std::endl;
        exit(1);
    }

    void executor::execute(int argc, char** argv) const
    {
        auto factory = factory::get();
        auto args = factory->get_args();
        args->parse(argc, argv);
        std::string path_to_nodes, path_to_edges;
        if (!args->input_file_path(path_to_nodes, path_to_edges))
            args->help();
        auto input = factory->get_input();
        const auto remove = args->remove_first_line();
        if (!input->from_csv_file(path_to_nodes, remove))
            print_and_exit("Failed to read edge data from file.");
        const auto nodes = input->get_mat();
        if (!input->from_csv_file(path_to_edges, remove))
            print_and_exit("Failed to read edge data from file.");
        const auto edges = input->get_mat();
        auto calc = factory->get_calc();
        calc->init(nodes, edges, args->max_iterations(), args->accuracy());
        calc->node_admittance();
        const auto node_adm_real = calc->get_node_adm_real();
        const auto node_adm_imag = calc->get_node_adm_imag();
        const auto verbose = args->verbose();
        if (verbose) {
            std::cout << "Real part of node admittance matrix:" << std::endl;
            output::print_dmat(*node_adm_real);
            std::cout << "Imaginary part of node admittance matrix:" << std::endl;
            output::print_dmat(*node_adm_imag);
        }
        std::string output_path;
        if (!args->output_file_path(output_path))
            std::cout << "Output file path not specified. Using result-*.csv by default." << std::endl;

    }
}
