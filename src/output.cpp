#include <iomanip>

#include "output.hpp"
#include "executor.hpp"

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif // _WIN32


namespace flow
{
    int output::max_elems_per_line()
    {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
            return 10;
        const auto width = csbi.dwSize.X;
#else
        winsize win;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
        const auto width = win.ws_col;
#endif // _WIN32
        return std::floor((width - 7) / 11);
    }

    void output::print_dmat(const arma::dmat& mat)
    {
        mat.each_row([](const arma::rowvec& row)
        {
            std::cout << std::setprecision(7) << std::left;
            auto counter = 0;
            auto elems = max_elems_per_line();
            for (auto&& elem : row)
            {
                if (++counter > elems) {
                    std::cout << "...(" << row.n_elem - elems << ')';
                    break;
                }
                std::cout << std::setw(10) << elem;
            }
            std::cout << std::endl;
        });
    }
}
