//
// arma-flow/writer.cpp
//
// @author CismonX
//

#include "writer.hpp"

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/ioctl.h>
#endif // _WIN32
#include <iomanip>
#include <experimental/filesystem>

namespace flow
{
    int writer::max_elems_per_line()
    {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            return 10;
        }
        const auto width = csbi.dwSize.X;
#else
        winsize win;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) == -1) {
            return 10;
        }
        const auto width = win.ws_col;
#endif // _WIN32
        return std::floor((width - 7) / 11);
    }

    std::string writer::double_to_string(double val)
    {
        auto str = std::to_string(val ? val : std::abs(val));
        auto pos = str.find_last_not_of('0');
        if (pos == 0 || str[pos] == '.') {
            --pos;
        }
        return str.substr(0, pos + 1);
    }

    void writer::print_mat(const arma::mat& mat)
    {
        mat.each_row([](const arma::rowvec& row)
        {
            std::cout << std::left;
            auto counter = 0;
            auto elems = max_elems_per_line();
            for (auto&& elem : row) {
                if (++counter > elems) {
                    std::cout << "...(" << row.n_elem - elems << ')';
                    break;
                }
                std::cout << std::setw(10) << double_to_string(elem) << ' ';
            }
            std::cout << std::endl;
        });
    }

    void writer::print_complex(const std::string& prefix, const std::complex<double>& complex)
    {
        std::cout << prefix << complex.real() << (complex.imag() < 0 ? '-' : '+') <<
            'j' << std::abs(complex.imag()) << std::endl;
    }

    void writer::to_csv_file(const std::string& path, const arma::mat& mat, const std::string& header) const
    {
        std::ofstream ofstream;
        ofstream.exceptions(std::ifstream::failbit);
        namespace fs = std::experimental::filesystem;
        try {
            const auto real_path = output_path_prefix_ + path;
            ofstream.open(
#ifdef _WIN32
                real_path[1] == ':'
#else
                real_path[0] == '/'
#endif // _WIN32
                ? real_path : fs::current_path().string() + '/' + real_path);
            if (header.length())
                ofstream << header << std::endl;
            mat.each_row([&ofstream](const arma::rowvec& row)
            {
                for (auto col = 0U; col < row.n_elem; ++col) {
                    ofstream << double_to_string(row[col]);
                    if (col != row.n_elem - 1) {
                        ofstream << ',';
                    }
                }
                ofstream << std::endl;
            });
        }
        catch (const std::exception&) {
            error("Failed to write to file.");
        }
    }
}
