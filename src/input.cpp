#include <experimental/filesystem>

#include "input.hpp"

namespace flow
{
    bool input::from_csv_file(const std::string& path, bool remove_first_line)
    {
        std::ifstream ifstream;
        ifstream.exceptions(std::ifstream::failbit);
        namespace fs = std::experimental::filesystem;
        try {
            ifstream.open(path[0] == '/' ? path : fs::current_path().string() + '/' + path);
            if (remove_first_line)
                ifstream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        catch (const std::exception&) {
            return false;
        }
        return do_read(ifstream);
    }

    const arma::dmat& input::get_mat() const
    {
        return mat_;
    }
}
