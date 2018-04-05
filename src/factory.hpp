#pragma once

#include "input.hpp"
#include "args.hpp"
#include "executor.hpp"
#include "calc.hpp"
#include "output.hpp"

namespace flow
{
    class factory
    {
        input input_;

        args args_;

        executor executor_;

        calc calc_;

        output output_;

        static factory singleton_;

        explicit factory() = default;
        
    public:

        static factory* get()
        {
            return &singleton_;
        }

        input* get_input()
        {
            return &input_;
        }

        args* get_args()
        {
            return &args_;
        }

        executor* get_executor()
        {
            return &executor_;
        }

        calc* get_calc()
        {
            return &calc_;
        }

        output* get_output()
        {
            return &output_;
        }
    };

    inline factory factory::singleton_;
}