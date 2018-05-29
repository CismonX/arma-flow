//
// arma-flow/reader.hpp
//
// @author CismonX
//

#pragma once

#include <armadillo>

namespace flow
{
    /// Provides read utilities.
    class reader
    {
        /// The loaded matrix.
        arma::mat mat_;

        /**
         * Read matrix from an istream.
         * 
         * @param istream The istream to be read.
         * @return Whether read is successful.
         */
        bool do_read(std::istream& istream)
        {
            mat_.clear();
            return mat_.load(istream, arma::csv_ascii);
        }

    public:
        /**
         * Read matrix from a CSV file. The first line will be ignored.
         * 
         * @param path Path to file.
         * @param remove_first_line Whether to remove the first line of CSV file.
         * @return Whether the file is successfully read.
         */
        bool from_csv_file(const std::string& path, bool remove_first_line);
        
        /**
         * Get loaded matrix.
         */
        const arma::mat& get_mat() const;
    };
}