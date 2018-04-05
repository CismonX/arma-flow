#include "factory.hpp"

int main(int argc, char** argv)
{
    flow::factory::get()->get_executor()->execute(argc, argv);
}