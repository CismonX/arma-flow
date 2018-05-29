//
// arma-flow/reader.hpp
//
// @author CismonX
//

#include <experimental/filesystem>

#include "reader.hpp"

namespace flow
{
    bool reader::from_csv_file(const std::string& path, bool remove_first_line)
    {
        std::ifstream ifstream;
        ifstream.exceptions(std::ifstream::failbit);
        namespace fs = std::experimental::filesystem;
        try {
            ifstream.open(
#ifdef _WIN32
                path[1] == ':'
#else
                path[0] == '/'
#endif // _WIN32
                ? path : fs::current_path().string() + '/' + path);
            if (remove_first_line)
                ifstream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return do_read(ifstream);
        }
        catch (const std::exception&) {
            return false;
        }
    }

    const arma::mat& reader::get_mat() const
    {
        return mat_;
    }
}
