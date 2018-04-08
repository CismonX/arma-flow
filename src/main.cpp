//
// arma-flow/main.cpp
//
// @author CismonX
//

#include "factory.hpp"

/// Bootstrap the program.
int main(int argc, char** argv)
{
    flow::factory::get()->get_executor()->execute(argc, argv);
}
